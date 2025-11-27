#include "Graphics/RenderCommand.h"
#include "Graphics/Renderer.h"
#include "Core/Base.h"

#include <SDL3/SDL_mouse.h>
#include <glad/glad.h>

namespace Charm
{
    namespace Graphics
    {
        namespace RenderCommand
        {
            void Clear()
            {
                const glm::vec3& clearColor = Renderer::GetClearColor();
                glClearColor(V3_OPEN(clearColor), 1.f);
                glClear(GL_COLOR_BUFFER_BIT);
            }

            void SetViewport(u32 x, u32 y, u32 width, u32 height) { glViewport(x, y, width, height); }
            void SetLineWidth(float width) { glLineWidth(width); }
            void ShowCursor() { SDL_ShowCursor(); }
            void HideCursor() { SDL_HideCursor(); }
            void DrawArrays(PrimitiveType type, u32 vertexCount) { glDrawArrays(type, 0, vertexCount); }
            void DrawIndexed(PrimitiveType type, u64 indexCount) { glDrawElements(type, indexCount, GL_UNSIGNED_INT, NULL); }
        }
    }
}
