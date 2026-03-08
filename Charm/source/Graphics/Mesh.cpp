#include "Graphics/Mesh.h"
#include "Graphics/RendererInternals.h"

#include <glad/glad.h>
#include <algorithm>
#include <iterator>

namespace Charm
{
    namespace Graphics
    {
        namespace Meshes
        {
            Mesh GenerateCube()
            {
                Mesh cube;

                // Cube mesh data - 24 vertices (4 per face for proper normals/UVs)
                MeshVertex cubeVertices[24] = {
                    // Front face (Z+)
                    (MeshVertex){glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
                    (MeshVertex){glm::vec3(0.5f, -0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
                    (MeshVertex){glm::vec3(0.5f, 0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
                    (MeshVertex){glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f)},

                    // Right face (X+)
                    (MeshVertex){glm::vec3(0.5f, -0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, -0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, 0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, 0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f)},

                    // Back face (Z-)
                    (MeshVertex){glm::vec3(0.5f, -0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
                    (MeshVertex){glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
                    (MeshVertex){glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
                    (MeshVertex){glm::vec3(0.5f, 0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f)},

                    // Left face (X-)
                    (MeshVertex){glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
                    (MeshVertex){glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
                    (MeshVertex){glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},
                    (MeshVertex){glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f)},

                    // Top face (Y+)
                    (MeshVertex){glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, 0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, 0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
                    (MeshVertex){glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},

                    // Bottom face (Y-)
                    (MeshVertex){glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, -0.5f, -0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
                    (MeshVertex){glm::vec3(0.5f, -0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)},
                    (MeshVertex){glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)}};

                // Cube indices - 36 indices (6 faces * 2 triangles * 3 vertices)
                u32 cubeIndices[36] = {
                    0, 1, 2, 2, 3, 0,       // Front
                    4, 5, 6, 6, 7, 4,       // Right
                    8, 9, 10, 10, 11, 8,    // Back
                    12, 13, 14, 14, 15, 12, // Left
                    16, 17, 18, 18, 19, 16, // Top
                    20, 21, 22, 22, 23, 20  // Bottom
                };

                Validate(cube, cubeVertices, LEN(cubeVertices), cubeIndices, LEN(cubeIndices));
                return cube;
            }

            void Validate(Mesh& mesh, MeshVertex* vertices, u32 vertexCount, u32* indices, u32 indexCount)
            {
                mesh.vertexArray = VertexArray::Create();
                mesh.vertexBuffer = VertexBuffer::Create();
                mesh.indexBuffer = IndexBuffer::Create();

                mesh.vertices.reserve(vertexCount);
                mesh.indices.reserve(indexCount);

                std::copy(vertices, vertices + vertexCount, std::back_inserter(mesh.vertices));
                std::copy(indices, indices + indexCount, std::back_inserter(mesh.indices));

                VertexArray::Bind(mesh.vertexArray);

                VertexBuffer::Bind(mesh.vertexBuffer);
                VertexBuffer::SetData(mesh.vertices.size() * sizeof(MeshVertex), mesh.vertices.data(), GL_STATIC_DRAW);

                IndexBuffer::Bind(mesh.indexBuffer);
                IndexBuffer::SetData(mesh.indices.size() * sizeof(u32), mesh.indices.data(), GL_STATIC_DRAW);

                VertexArray::EnableAttributeLocation(0);
                VertexArray::EnableAttributeLocation(1);
                VertexArray::EnableAttributeLocation(2);
                VertexArray::EnableAttributeLocation(3);

                VertexArray::SpecifyFormat(0, 3, GL_FLOAT, sizeof(MeshVertex), offsetof(MeshVertex, position));
                VertexArray::SpecifyFormat(1, 4, GL_FLOAT, sizeof(MeshVertex), offsetof(MeshVertex, color));
                VertexArray::SpecifyFormat(2, 2, GL_FLOAT, sizeof(MeshVertex), offsetof(MeshVertex, texCoord));
                VertexArray::SpecifyFormat(3, 3, GL_FLOAT, sizeof(MeshVertex), offsetof(MeshVertex, normal));

                IndexBuffer::Unbind();
                VertexBuffer::Unbind();
                VertexArray::Unbind();
            }

            void SetupInstanceBuffer(Mesh& mesh, u32 maxInstances)
            {
                mesh.instanceBuffer = VertexBuffer::Create();

                VertexArray::Bind(mesh.vertexArray);
                VertexBuffer::Bind(mesh.instanceBuffer);
                VertexBuffer::SetData(maxInstances * sizeof(InstanceData), NULL, GL_STREAM_DRAW);

                for (u32 i = 0; i < 4; i++)
                {
                    const u32 location = 4 + i;
                    VertexArray::EnableAttributeLocation(location);
                    VertexArray::SpecifyFormat(location, 4, GL_FLOAT, sizeof(InstanceData), i * sizeof(glm::vec4));
                    VertexArray::SetAttributeDivisor(location, 1);
                }

                VertexArray::EnableAttributeLocation(8);
                VertexArray::SpecifyFormat(8, 1, GL_INT, sizeof(InstanceData), sizeof(glm::mat4));
                VertexArray::SetAttributeDivisor(8, 1);

                VertexArray::Unbind();
                VertexBuffer::Unbind();
            }

            void UploadInstanceData(const Mesh& mesh, const InstanceData* data, u32 count)
            {
                VertexBuffer::Bind(mesh.instanceBuffer);
                VertexBuffer::SubData(0, count * sizeof(InstanceData), data);
                VertexBuffer::Unbind();
            }

            void Unload(Mesh& mesh)
            {
                mesh.vertices.clear();
                mesh.indices.clear();

                if (mesh.vertexArray != 0)
                {
                    VertexArray::Destroy(mesh.vertexArray);
                    VertexBuffer::Destroy(mesh.vertexBuffer);
                    IndexBuffer::Destroy(mesh.indexBuffer);
                }

                if (mesh.ownsGPUResources && mesh.instanceBuffer != 0)
                    VertexBuffer::Destroy(mesh.instanceBuffer);
            }
        }
    }
}
