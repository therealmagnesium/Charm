#pragma once
#include "Graphics/Renderer.h"
#include "Core/Base.h"
#include <glad/glad.h>

namespace Charm
{
    namespace Graphics
    {
        namespace RenderCommand
        {
            inline void Clear()
            {
                const glm::vec3& clearColor = Renderer::GetClearColor();
                glClearColor(V3_OPEN(clearColor), 1.f);
                glClear(GL_COLOR_BUFFER_BIT);
            }

            inline void SetViewport(u32 x, u32 y, u32 width, u32 height)
            {
                glViewport(x, y, width, height);
            }
        }
    }
}
