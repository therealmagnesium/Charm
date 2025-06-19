#include "SceneViewport.h"

#include <Charm.h>
#include <imgui.h>

using namespace Charm::Graphics;

namespace Charm
{
    static SceneViewportState state;

    namespace SceneViewportPanel
    {
        ImVec2 GetLargestViewportSize();
        ImVec2 GetCenteredViewportPosition(ImVec2& aspectSize);

        void Display(Framebuffer& framebuffer)
        {
            ImGui::Begin("Scene Viewport", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            state.isHovered = ImGui::IsWindowHovered();
            state.isFocused = ImGui::IsWindowFocused();

            ImVec2 aspectSize = GetLargestViewportSize();
            ImVec2 windowPosition = GetCenteredViewportPosition(aspectSize);
            ImTextureID textureID = framebuffer.attachments[0].id;

            state.position = glm::vec2(ImGui::GetWindowPos().x + windowPosition.x, ImGui::GetWindowPos().y + windowPosition.y);
            state.size = *(glm::vec2*)&aspectSize;

            ImGui::SetCursorPos(windowPosition);
            ImGui::Image(textureID, aspectSize, ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
            ImGui::End();
        }

        ImVec2 GetLargestViewportSize()
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            ImVec2 aspectSize = ImVec2(windowSize.x, windowSize.x / (16.f / 9.f));

            if (aspectSize.y > windowSize.y)
            {
                aspectSize.y = windowSize.y;
                aspectSize.x = windowSize.y * (16.f / 9.f);
            }

            return aspectSize;
        }

        ImVec2 GetCenteredViewportPosition(ImVec2& aspectSize)
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            ImVec2 viewport;
            viewport.x = (windowSize.x / 2.f) - (aspectSize.x / 2.f) + ImGui::GetCursorPosX();
            viewport.y = (windowSize.y / 2.f) - (aspectSize.y / 2.f) + ImGui::GetCursorPosY();

            return viewport;
        }

        bool IsHovered() { return state.isHovered; }
        bool IsFocused() { return state.isFocused; }
        const glm::vec2& GetPosition() { return state.position; }
        const glm::vec2& GetSize() { return state.size; }
    }
}
