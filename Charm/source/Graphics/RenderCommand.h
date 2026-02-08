#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace Graphics
    {
        enum class PrimitiveType : u8
        {
            Points = 0,
            Lines,
            LineLoop,
            LineStrip,
            Triangles,
            TriangleStrip,
            TriangleFan
        };

        enum class BufferFunc : u8
        {
            Never = 0,
            Less,
            Equal,
            LessOrEqual,
            Greater,
            NotEqual,
            GreaterOrEqual,
            Always
        };

        enum class StencilOperation : u8
        {
            Keep = 0,
            Replace
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
            void EnableDepthBuffer();
            void DisableDepthBuffer();
            void SetDepthFunc(BufferFunc func);
            void EnableStencilWriting();
            void DisableStencilWriting();
            void EnableStencilBuffer();
            void DisableStencilBuffer();
            void SetStencilFunc(BufferFunc func, u32 reference, u32 mask);
            void SetStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation bothPass);
            void DrawArrays(PrimitiveType type, u32 vertexCount);
            void DrawIndexed(PrimitiveType type, u64 indexCount);
        }
    }
}
