#include "DebugStatsPanel.h"
#include "SceneViewport.h"
#include "../CharmApp.h"

#include <Core/Application.h>
#include <Core/Input.h>
#include <Core/Time.h>
#include <Core/Utils.h>

#include <Graphics/Renderer.h>

#include <imgui.h>

using namespace Charm;
using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static DebugStatsState state;

    namespace DebugStatsPanel
    {
        void Display()
        {
            const ApplicationConfig& config = Application::GetConfig();
            const glm::vec2 virtualMouse = Utils::ScreenToVirtual(Input::GetMousePosition());
            const glm::vec2 viewportMouse = Utils::ScreenToViewport(Input::GetMousePosition(),
                                                                    SceneViewportPanel::GetPosition(),
                                                                    SceneViewportPanel::GetSize());
            const glm::vec2 glViewportMouse = Utils::ScreenToViewportGL(Input::GetMousePosition(),
                                                                        SceneViewportPanel::GetPosition(),
                                                                        SceneViewportPanel::GetSize());
            const s32 pixelData = CharmApp::GetPixelData();
            const Scene* activeScene = CharmApp::GetActiveScene();

            ImGui::Begin("Debug Stats");
            ImGui::Text("FPS: %d", (u32)(1.f / Time::GetDelta()));
            ImGui::Text("MS per frame: %.7f", Time::GetDelta());
            ImGui::Text("Number of quads: %d", Renderer::GetQuadCount());
            ImGui::Text("Number of circles: %d", Renderer::GetCircleCount());
            ImGui::Text("Number of draw calls: %d", Renderer::GetDrawCount());
            ImGui::Text("Editor camera distance: %.2f", activeScene->editorCamera3D.distance);
            ImGui::Text("Pixel data: %d", pixelData);
            ImGui::Text("Virtual mouse position: " V2_FMT, V2_OPEN(virtualMouse));
            ImGui::Text("Viewport mouse position: " V2_FMT, V2_OPEN(viewportMouse));
            ImGui::Text("GL mouse position: " V2_FMT, V2_OPEN(glViewportMouse));
            ImGui::Text("Is viewport hovered?: %s", Utils::BoolToCString(SceneViewportPanel::IsHovered()));
            ImGui::Text("Is viewport focused?: %s", Utils::BoolToCString(SceneViewportPanel::IsFocused()));
            ImGui::End();
        }
    }
}
