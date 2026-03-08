#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace Graphics
    {
        namespace VertexArray
        {
            u32 Create();
            void Destroy(u32& vao);
            void Bind(u32 vao);
            void Unbind();
            void EnableAttributeLocation(u32 location);
            void DisableAttributeLocation(u32 location);
            void SpecifyFormat(u32 location, u32 numComponents, u32 type, u64 stride, u64 offset);
            void SetAttributeDivisor(u32 location, u32 divisor);
        }

        namespace VertexBuffer
        {
            u32 Create();
            void Destroy(u32& vbo);
            void Bind(u32 vbo);
            void Unbind();
            void SetData(u64 size, const void* data, u32 usage);
            void SubData(u64 offset, u64 size, const void* data);
        }

        namespace IndexBuffer
        {
            u32 Create();
            void Destroy(u32& ebo);
            void Bind(u32 ebo);
            void Unbind();
            void SetData(u64 size, const u32* data, u32 usage);
        }
    }
}
