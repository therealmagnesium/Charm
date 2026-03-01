#include "Graphics/RenderAPI.h"
#include "Graphics/Renderer.h"
#include "Core/Base.h"
#include "Projects/Project.h"

#include <SDL3/SDL_mouse.h>
#include <glad/glad.h>

using namespace Charm::Projects;

namespace Charm
{
    namespace Graphics
    {
        namespace RenderAPI
        {
            void Clear()
            {
                const Project& project = ProjectManager::GetActive();
                const glm::vec3& clearColor = Renderer::GetClearColor();
                const u32 clearBuffer = project.type == ProjectType::ThreeDimensional ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT : GL_COLOR_BUFFER_BIT;
                glClearColor(V3_OPEN(clearColor), 1.f);
                glClearStencil(0);
                glClear(clearBuffer);
            }

            void SetViewport(u32 x, u32 y, u32 width, u32 height) { glViewport(x, y, width, height); }
            void SetLineWidth(float width) { glLineWidth(width); }
            void ShowCursor() { SDL_ShowCursor(); }
            void HideCursor() { SDL_HideCursor(); }
            void EnableDepthWriting() { glDepthMask(true); }
            void DisableDepthWriting() { glDepthMask(false); }
            void EnableDepthBuffer() { glEnable(GL_DEPTH_TEST); }
            void DisableDepthBuffer() { glDisable(GL_DEPTH_TEST); }
            void SetDepthFunc(BufferFunc func) { glDepthFunc(0x200 + (u8)func); }
            void EnableStencilBuffer() { glEnable(GL_STENCIL_TEST); }
            void DisableStencilBuffer() { glDisable(GL_STENCIL_TEST); }
            void EnableStencilWriting() { glStencilMask(0xFF); }
            void DisableStencilWriting() { glStencilMask(0x00); }
            void SetStencilFunc(BufferFunc func, u32 reference, u32 mask) { glStencilFunc(0x200 + (u8)func, reference, mask); }
            void SetStencilOperation(StencilOperation stencilFail, StencilOperation depthFail, StencilOperation bothPass) { glStencilOp(0x1E00 + (u8)stencilFail, 0x1E00 + (u8)depthFail, 0x1E00 + (u8)bothPass); }
            void DrawArrays(PrimitiveType type, u32 vertexCount) { glDrawArrays((u8)type, 0, vertexCount); }
            void DrawIndexed(PrimitiveType type, u64 indexCount) { glDrawElements((u8)type, indexCount, GL_UNSIGNED_INT, NULL); }
        }
    }
}
