#include "ToolbarPanel.h"
#include "../CharmApp.h"

#include <imgui.h>
#include <ImGuizmo.h>

namespace CharmApp
{
    static ToolbarState state;

    namespace ToolbarPanel
    {
        void Init()
        {
            state.icons[(u32)ToolbarIcon::Play] = Textures::Load("assets/textures/button_play.png");
            state.icons[(u32)ToolbarIcon::Stop] = Textures::Load("assets/textures/button_stop.png");
            state.icons[(u32)ToolbarIcon::Translate] = Textures::Load("assets/textures/button_translate.png");
            state.icons[(u32)ToolbarIcon::Rotate] = Textures::Load("assets/textures/button_rotate.png");
            state.icons[(u32)ToolbarIcon::Scale] = Textures::Load("assets/textures/button_scale.png");
            state.windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
            state.manipulationType = ImGuizmo::OPERATION::TRANSLATE;
        }

        void Shutdown()
        {
            for (Texture& icon : state.icons)
                Textures::Unload(icon);
        }

        void Display()
        {
            const SceneState sceneState = CharmApp::GetActiveSceneState();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));

            const auto& colors = ImGui::GetStyle().Colors;
            const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
            const auto& buttonActive = colors[ImGuiCol_ButtonActive];
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(V3_OPEN(buttonHovered), 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(V3_OPEN(buttonActive), 0.5f));

            ImGui::Begin("Toolbar", NULL, state.windowFlags);

            float maxIconSize = 40.f;
            float iconSize = ImGui::GetWindowHeight() - 12.f;

            if (Window::IsMaximized())
                maxIconSize = 65.f;

            if (iconSize > maxIconSize)
                iconSize = maxIconSize;

            if (ImGui::ImageButton("##TranslateButton", state.icons[(u32)ToolbarIcon::Translate].id, ImVec2(iconSize, iconSize)))
                state.manipulationType = ImGuizmo::OPERATION::TRANSLATE;
            ImGui::SameLine();
            if (ImGui::ImageButton("##RotateButton", state.icons[(u32)ToolbarIcon::Rotate].id, ImVec2(iconSize, iconSize)))
                state.manipulationType = ImGuizmo::OPERATION::ROTATE;
            ImGui::SameLine();
            if (ImGui::ImageButton("##ScaleButton", state.icons[(u32)ToolbarIcon::Scale].id, ImVec2(iconSize, iconSize)))
                state.manipulationType = ImGuizmo::OPERATION::SCALE;
            ImGui::SameLine();

            ImTextureID runtimeIcon = (sceneState == SceneState::Editor) ? state.icons[(u32)ToolbarIcon::Play].id : state.icons[(u32)ToolbarIcon::Stop].id;
            ImGui::SetCursorPosX((ImGui::GetContentRegionMax().x * 0.5f) - (iconSize * 0.5f));
            if (ImGui::ImageButton("##PlayButton", runtimeIcon, ImVec2(iconSize, iconSize)))
            {
                if (sceneState == SceneState::Editor)
                    OnScenePlay();
                else
                    OnSceneStop();
            }

            ImGui::End();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
        }

        u32 GetManipulationType() { return state.manipulationType; }
        const Texture& GetIcon(ToolbarIcon type) { return state.icons[(u32)type]; }
        void SetManipulationType(u32 type) { state.manipulationType = type; }
    }
}
