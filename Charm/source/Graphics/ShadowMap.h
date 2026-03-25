#pragma once
#include "Core/Base.h"

#include <glm/glm.hpp>

namespace Charm::Graphics
{
    // Number of cascade splits. 4 is industry standard for open world titles.
    // More cascades = smoother quality transitions but more shadow passes per frame.
    constexpr u32 k_CSMCascadeCount = 4;

    // Depth texture resolution per cascade. 2048 is a strong quality/perf balance.
    // For high-end targets raise to 4096; for mobile drop to 1024.
    constexpr u32 k_ShadowMapResolution = 2048;

    // Controls the blend between logarithmic (1.0) and uniform (0.0) cascade splits.
    // 0.75 gives sharp near shadows while keeping acceptable quality in the distance.
    constexpr float k_CSMLambda = 0.75f;

    // Extra depth range added behind the light frustum so geometry behind the camera
    // (tall buildings, hills) still casts shadows into the visible scene.
    constexpr float k_ShadowZMultiplier = 10.f;

    // -------------------------------------------------------------------------
    struct ShadowCascade
    {
        // Full light-space MVP matrix for this cascade.
        glm::mat4 lightSpaceMatrix = glm::mat4(1.f);

        // Positive view-space depth (abs(z)) of the FAR plane of this cascade.
        // Used in the fragment shader to select which cascade a fragment belongs to.
        float splitDepth = 0.f;
    };

    // -------------------------------------------------------------------------
    // All GPU state for the cascaded shadow map.
    // One GL_TEXTURE_2D_ARRAY holds all cascade depth layers;
    // one FBO per cascade layer avoids layered rendering complexity.
    struct ShadowMap
    {
        u32 fbos[k_CSMCascadeCount] = {};
        u32 depthTextureArray = 0; // GL_TEXTURE_2D_ARRAY, k_CSMCascadeCount layers
        ShadowCascade cascades[k_CSMCascadeCount];
        bool isValid = false;
    };

    // -------------------------------------------------------------------------
    namespace ShadowMaps
    {
        // Allocates GPU resources (texture array + FBOs). Call once at startup.
        ShadowMap Create();

        // Frees all GPU resources. Call at shutdown.
        void Destroy(ShadowMap& shadow);

        // Recomputes all cascade light-space matrices for the current frame.
        // viewMatrix / projMatrix  — camera matrices from Renderer::GetViewMatrix/Projection.
        // nearClip / farClip       — camera clip planes.
        // lightDirection           — world-space normalised direction TOWARD the sun.
        //                           (i.e. the negated sun.direction stored in DirectionalLight)
        void UpdateCascades(ShadowMap& shadow,
                            const glm::mat4& viewMatrix,
                            const glm::mat4& projMatrix,
                            float nearClip,
                            float farClip,
                            const glm::vec3& lightDirection);
    }
}
