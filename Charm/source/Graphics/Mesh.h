#pragma once
#include "Core/Base.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct MeshVertex
        {
            glm::vec3 position;
            glm::vec4 color;
            glm::vec2 texCoord;
            glm::vec3 normal;
        };

        struct Material
        {
            glm::vec4 albedo = glm::vec4(1.f);
            Texture* albedoTexture = NULL;
            Shader* shader = NULL;

            inline bool IsTranslucent() const
            {
                const bool colorHasTransparency = albedo.a < 1.f;
                const bool textureHasTransparency = albedoTexture != NULL && albedoTexture->hasTransparency;
                return colorHasTransparency || textureHasTransparency;
            }
        };

        // Per-instance data streamed into the instance buffer each frame.
        // transform occupies attribute locations 4–7; entityID occupies location 8.
        struct InstanceData
        {
            glm::mat4 transform = glm::mat4(1.f);
            s32 entityID = -1;
        };

        struct Mesh
        {
            std::vector<MeshVertex> vertices;
            std::vector<u32> indices;
            u32 vertexArray = 0;
            u32 vertexBuffer = 0;
            u32 indexBuffer = 0;
            u32 instanceBuffer = 0;
            s32 materialIndex = -1;
            bool ownsGPUResources = true;

            inline glm::vec3 GetCenter() const
            {
                if (vertices.empty()) return glm::vec3(0.f);

                glm::vec3 min(FLT_MAX), max(-FLT_MAX);
                for (const auto& vertex : vertices)
                {
                    min = glm::min(min, vertex.position);
                    max = glm::max(max, vertex.position);
                }
                return (min + max) * 0.5f;
            }
        };

        namespace Meshes
        {
            Mesh GenerateCube();
            void Validate(Mesh& mesh, MeshVertex* vertices, u32 vertexCount, u32* indices, u32 indexCount);
            void SetupInstanceBuffer(Mesh& mesh, u32 maxInstances);
            void UploadInstanceData(const Mesh& mesh, const InstanceData* data, u32 count);
            void Unload(Mesh& mesh);
        }
    }
}
