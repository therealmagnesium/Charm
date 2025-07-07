#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace Graphics
    {
        namespace RenderCommand
        {
            void Clear();
            void SetViewport(u32 x, u32 y, u32 width, u32 height);
            void SetLineWidth(float width);
            void ShowCursor();
            void HideCursor();
        }
    }
}
