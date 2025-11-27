#include "Graphics/RendererInternals.h"
#include <glad/glad.h>

namespace Charm
{
    namespace Graphics
    {
        namespace VertexArray
        {
            u32 Create()
            {
                u32 vao = 0;
                glGenVertexArrays(1, &vao);

                return vao;
            }

            void Destroy(u32& vao) { glDeleteVertexArrays(1, &vao); }
            void Bind(u32 vao) { glBindVertexArray(vao); }
            void Unbind() { glBindVertexArray(0); }
            void EnableAttributeLocation(u32 location) { glEnableVertexAttribArray(location); }
            void DisableAttributeLocation(u32 location) { glDisableVertexAttribArray(location); }
            void SpecifyFormat(u32 location, u32 numComponents, u32 type, u64 stride, u64 offset) { glVertexAttribPointer(location, numComponents, type, false, stride, (void*)offset); }
        }

        namespace VertexBuffer
        {
            u32 Create()
            {
                u32 vbo = 0;
                glGenBuffers(1, &vbo);

                return vbo;
            }

            void Destroy(u32& vbo) { glDeleteBuffers(1, &vbo); }
            void Bind(u32 vbo) { glBindBuffer(GL_ARRAY_BUFFER, vbo); }
            void SetData(u64 size, const float* data, u32 usage) { glBufferData(GL_ARRAY_BUFFER, size, data, usage); }
        }

        namespace IndexBuffer
        {
            u32 Create()
            {
                u32 ebo = 0;
                glGenBuffers(1, &ebo);

                return ebo;
            }

            void Destroy(u32& ebo) { glDeleteBuffers(1, &ebo); }
            void Bind(u32 ebo) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); }
            void SetData(u64 size, const u32* data, u32 usage) { glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage); }
        }
    }
}
