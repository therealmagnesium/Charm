#include "Graphics/ShadowMap.h"
#include "Core/Log.h"

#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <algorithm>
#include <cmath>

namespace Charm::Graphics
{
    namespace ShadowMaps
    {
        // -----------------------------------------------------------------------
        // Internal helpers
        // -----------------------------------------------------------------------

        // Returns the 8 world-space corners of the camera sub-frustum defined by
        // [nearZ, farZ] in NDC, reconstructed from an inverse view-projection.
        static std::array<glm::vec4, 8> GetFrustumCornersWorldSpace(
            const glm::mat4& viewProj, float nearZ, float farZ,
            const glm::mat4& fullViewProj)
        {
            // We build a sub-frustum by scaling the projection NDC z-range.
            // Decompose the projection to get the full near/far, rebuild a
            // projection whose z-range is [nearZ, farZ], then invert.
            //
            // The robust approach: just supply a view-projection built from
            // the sub-frustum projection matrix directly (done in UpdateCascades).
            const glm::mat4 inv = glm::inverse(viewProj);

            std::array<glm::vec4, 8> corners;
            u32 idx = 0;
            for (u32 x = 0; x < 2; ++x)
                for (u32 y = 0; y < 2; ++y)
                    for (u32 z = 0; z < 2; ++z)
                    {
                        const glm::vec4 ndc = glm::vec4(
                            2.f * (float)x - 1.f,
                            2.f * (float)y - 1.f,
                            2.f * (float)z - 1.f,
                            1.f);
                        glm::vec4 world = inv * ndc;
                        world /= world.w;
                        corners[idx++] = world;
                    }
            return corners;
        }

        ShadowMap Create()
        {
            ShadowMap shadow;

            // --- Depth texture array -------------------------------------------
            glGenTextures(1, &shadow.depthTextureArray);
            glBindTexture(GL_TEXTURE_2D_ARRAY, shadow.depthTextureArray);

            glTexImage3D(
                GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
                k_ShadowMapResolution, k_ShadowMapResolution,
                k_CSMCascadeCount, 0,
                GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

            // Pixels outside the shadow map are lit (no shadow). Setting the border
            // colour to 1.0 (maximum depth) achieves this automatically.
            const float borderColor[] = {1.f, 1.f, 1.f, 1.f};
            glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

            // Enable hardware PCF on the depth comparison
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

            // --- One FBO per cascade -------------------------------------------
            glGenFramebuffers(k_CSMCascadeCount, shadow.fbos);
            for (u32 i = 0; i < k_CSMCascadeCount; ++i)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbos[i]);

                // Attach layer i of the array texture as the depth attachment
                glFramebufferTextureLayer(
                    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                    shadow.depthTextureArray, 0, (GLint)i);

                // Explicitly tell OpenGL there is no colour output
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    ERROR("ShadowMaps::Create - Cascade %u FBO is not complete!", i);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            shadow.isValid = true;
            INFO("ShadowMap created: %u cascades at %ux%u",
                 k_CSMCascadeCount, k_ShadowMapResolution, k_ShadowMapResolution);

            return shadow;
        }

        void Destroy(ShadowMap& shadow)
        {
            if (!shadow.isValid) return;

            glDeleteFramebuffers(k_CSMCascadeCount, shadow.fbos);

            if (shadow.depthTextureArray != 0)
                glDeleteTextures(1, &shadow.depthTextureArray);

            shadow = ShadowMap{};
            INFO("ShadowMap destroyed");
        }

        void UpdateCascades(ShadowMap& shadow,
                            const glm::mat4& viewMatrix,
                            const glm::mat4& projMatrix,
                            float nearClip,
                            float farClip,
                            const glm::vec3& lightDirection)
        {
            if (!shadow.isValid) return;

            // ------------------------------------------------------------------
            // 1. Compute cascade split depths using the practical split scheme:
            //    blend of logarithmic and uniform distributions.
            // ------------------------------------------------------------------
            float splitDepths[k_CSMCascadeCount + 1];
            splitDepths[0] = nearClip;

            const float range = farClip - nearClip;
            const float ratio = farClip / nearClip;

            for (u32 i = 1; i <= k_CSMCascadeCount; ++i)
            {
                const float p = (float)i / (float)k_CSMCascadeCount;
                const float logSplit = nearClip * std::pow(ratio, p);
                const float unifSplit = nearClip + range * p;
                splitDepths[i] = k_CSMLambda * logSplit + (1.f - k_CSMLambda) * unifSplit;
            }

            // ------------------------------------------------------------------
            // 2. For each cascade, fit a tight orthographic frustum to the
            //    camera sub-frustum corners, expressed in light space.
            // ------------------------------------------------------------------
            const glm::vec3 lightDir = glm::normalize(lightDirection);
            const glm::vec3 up = std::abs(glm::dot(lightDir, glm::vec3(0.f, 1.f, 0.f))) < 0.99f
                                     ? glm::vec3(0.f, 1.f, 0.f)
                                     : glm::vec3(1.f, 0.f, 0.f);

            for (u32 i = 0; i < k_CSMCascadeCount; ++i)
            {
                const float cascadeNear = splitDepths[i];
                const float cascadeFar = splitDepths[i + 1];

                // Build a perspective projection covering exactly this cascade range
                glm::mat4 cascadeProj = glm::perspective(
                    glm::radians(45.f),                                     // NOTE: caller should ideally pass fov
                    projMatrix[1][1] == 0.f ? 1.f : 1.f / projMatrix[1][1], // rough aspect
                    cascadeNear, cascadeFar);

                // The aspect ratio is baked into the projection matrix.
                // Extract it: for glm::perspective, proj[0][0] = 1/(aspect*tan(fov/2))
                //             and proj[1][1] = 1/tan(fov/2)
                // So aspect = proj[1][1] / proj[0][0].
                if (projMatrix[0][0] != 0.f)
                {
                    const float aspect = projMatrix[1][1] / projMatrix[0][0];
                    const float fov = 2.f * std::atan(1.f / projMatrix[1][1]);
                    cascadeProj = glm::perspective(fov, aspect, cascadeNear, cascadeFar);
                }

                // Get the 8 world-space corners of this sub-frustum
                const glm::mat4 cascadeVP = cascadeProj * viewMatrix;
                const auto corners = GetFrustumCornersWorldSpace(cascadeVP, cascadeNear, cascadeFar, cascadeVP);

                // Compute the centroid
                glm::vec3 centroid(0.f);
                for (const auto& c : corners)
                    centroid += glm::vec3(c);
                centroid /= (float)corners.size();

                // Build a light-view matrix looking from above the centroid
                const glm::mat4 lightView = glm::lookAt(
                    centroid + lightDir,
                    centroid,
                    up);

                // Compute a tight AABB in light space
                float minX = FLT_MAX, maxX = -FLT_MAX;
                float minY = FLT_MAX, maxY = -FLT_MAX;
                float minZ = FLT_MAX, maxZ = -FLT_MAX;

                for (const auto& c : corners)
                {
                    const glm::vec4 ls = lightView * c;
                    minX = std::min(minX, ls.x);
                    maxX = std::max(maxX, ls.x);
                    minY = std::min(minY, ls.y);
                    maxY = std::max(maxY, ls.y);
                    minZ = std::min(minZ, ls.z);
                    maxZ = std::max(maxZ, ls.z);
                }

                // Expand the Z range so geometry BEHIND the camera can still cast
                // shadows into the visible scene (tall trees, cliff faces, etc.)
                if (minZ < 0.f)
                    minZ *= k_ShadowZMultiplier;
                else
                    minZ /= k_ShadowZMultiplier;

                if (maxZ < 0.f)
                    maxZ /= k_ShadowZMultiplier;
                else
                    maxZ *= k_ShadowZMultiplier;

                // ------------------------------------------------------------------
                // Texel snapping: quantise the light-space AABB bounds to the texel
                // grid BEFORE building the ortho projection. This is the correct place
                // to snap — operating directly in world units, not in NDC, so the
                // quantization is exact and there is no floating-point rounding error
                // from the extra projection and round-trip.
                // ------------------------------------------------------------------
                {
                    const float worldUnitsPerTexel = (maxX - minX) / (float)k_ShadowMapResolution;
                    minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
                    maxX = std::floor(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
                    minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
                    maxY = std::floor(maxY / worldUnitsPerTexel) * worldUnitsPerTexel;
                }

                glm::mat4 lightProj = glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);

                shadow.cascades[i].lightSpaceMatrix = lightProj * lightView;

                // Store the cascade far-plane as a RAW NEGATIVE VIEW-SPACE depth.
                //
                // OpenGL view space has the camera looking down -Z, so a point
                // CascadeFar units in front of the camera has viewZ = -cascadeFar.
                //
                // The fragment shader computes:
                //   viewZ = (u_matrixView * vec4(worldPos, 1.0)).z
                //
                // After a view-matrix multiply the w component is always 1.0, so
                // no perspective divide is needed. Both sides of the comparison
                // are plain view-space Z values, e.g.:
                //   splitDepth[0] = -31.9
                //   fragment 5 units away → viewZ = -5 → -5 > -31.9 → cascade 0 ✓
                //   fragment 50 units away → viewZ = -50 → -50 > -31.9 → false → cascade 1+
                //
                // The previous (broken) approach stored NDC depth ≈ 0.994 while the
                // shader compared view-space Z ≈ -5. Since -5 > 0.994 is always
                // false, every fragment defaulted to cascade 3 (the far cascade),
                // which contained no nearby geometry, so all shadow lookups
                // returned the cleared depth of 1.0 → fully lit → no shadows.
                shadow.cascades[i].splitDepth = -cascadeFar;
            }
        }
    }
}
