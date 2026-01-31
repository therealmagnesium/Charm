#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace Graphics
    {
        enum PrimitiveType : u8
        {
            Points = 0,
            Lines,
            LineLoop,
            LineStrip,
            Triangles,
            TriangleStrip,
            TriangleFan
        };

        namespace RenderCommand
        {
            void Clear();
            void SetViewport(u32 x, u32 y, u32 width, u32 height);
            void SetLineWidth(float width);
            void ShowCursor();
            void HideCursor();
            void EnableDepthWriting();
            void DisableDepthWriting();
            void EnableDepthTest();
            void DisableDepthTest();
            void DrawArrays(PrimitiveType type, u32 vertexCount);
            void DrawIndexed(PrimitiveType type, u64 indexCount);
        }
    }
}
