#include "ToolbarPanel.h"
#include "../CharmApp.h"

#include <imgui.h>

namespace CharmApp
{
    static ToolbarState state;

    namespace ToolbarPanel
    {
        void Init()
        {
            state.iconPlay = Textures::Load("assets/textures/play_button.png");
            state.iconStop = Textures::Load("assets/textures/stop_button.png");
            state.windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        }

        void Shutdown()
        {
            Textures::Unload(state.iconPlay);
            Textures::Unload(state.iconStop);
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

            ImGui::Begin("##Toolbar", NULL, state.windowFlags);
            float iconSize = ImGui::GetWindowHeight() - 12.f;
            ImTextureID icon = (sceneState == SceneState::Editor) ? state.iconPlay.id : state.iconStop.id;
            ImGui::SetCursorPosX((ImGui::GetContentRegionMax().x * 0.5f) - (iconSize * 0.5f));
            if (ImGui::ImageButton("##PlayButton", icon, ImVec2(iconSize, iconSize), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f)))
            {
                if (sceneState == SceneState::Editor)
                    OnScenePlay();
                else
                    OnSceneStop();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);
            ImGui::End();
        }
    }
}
