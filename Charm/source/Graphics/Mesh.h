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

        struct Mesh
        {
            std::vector<MeshVertex> vertices;
            std::vector<u32> indices;
            u32 vertexArray = 0;
            u32 vertexBuffer = 0;
            u32 indexBuffer = 0;
            s32 materialIndex = -1;

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
            void Unload(Mesh& mesh);
        }
    }
}
