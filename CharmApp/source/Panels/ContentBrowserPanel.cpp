#include "ContentBrowserPanel.h"
#include "SceneHeirarchyPanel.h"
#include "InspectorPanel.h"
#include "../CharmApp.h"

#include <imgui.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    static ContentBrowserState state;

    namespace ContentBrowserPanel
    {
        void Init()
        {
            state.currentDirectory = ProjectManager::GetAssetPath(CharmApp::GetProject());
            state.iconFile = Textures::Load("assets/textures/file_icon.png");
            state.iconFolder = Textures::Load("assets/textures/folder_icon.png");
        }

        void Shutdown()
        {
            Textures::Unload(state.iconFile);
            Textures::Unload(state.iconFolder);
        }

        void Display()
        {
            ImGui::Begin("Content Browser");

            ImGui::PushID("Padding");
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 130.f);
            ImGui::Text("Padding");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.7f);
            ImGui::DragFloat("##Padding", &state.padding);
            ImGui::Columns(1);
            ImGui::PopID();

            ImGui::PushID("Thumbnail size");
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 130.f);
            ImGui::Text("Thumbnail size");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.7f);
            ImGui::DragFloat("##Thumbnail size", &state.thumbnailSize);
            ImGui::Columns(1);
            ImGui::PopID();

            ImGui::Separator();

            if (state.currentDirectory != ProjectManager::GetAssetPath(CharmApp::GetProject()))
            {
                if (ImGui::Button("Back"))
                {
                    state.currentDirectory = state.currentDirectory.parent_path();
                    state.selectedFilePath = "";
                }
            }

            if (SceneHeirarchyPanel::GetSelectedEntity())
                state.selectedFilePath = "";

            float cellSize = state.thumbnailSize + state.padding;
            float panelWidth = ImGui::GetContentRegionAvail().x;
            u32 columnCount = (u32)(panelWidth / cellSize);
            if (columnCount < 1)
                columnCount = 1;

            ImGui::Columns(columnCount, NULL, false);

            for (auto& entry : std::filesystem::directory_iterator(state.currentDirectory))
            {
                std::filesystem::path path = entry.path();
                std::string filename = path.filename().string();

                ImGui::PushID(path.c_str());

                ImTextureID icon = (entry.is_directory()) ? state.iconFolder.id : state.iconFile.id;
                ImVec2 buttonSize = ImVec2(state.thumbnailSize, state.thumbnailSize);

                const auto& colors = ImGui::GetStyle().Colors;
                const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
                const auto& buttonActive = colors[ImGuiCol_ButtonActive];
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(V3_OPEN(buttonHovered), 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(V3_OPEN(buttonActive), 0.5f));
                if (ImGui::ImageButton("##Icon", icon, buttonSize,
                                       ImVec2(0.f, 0.f), ImVec2(1.f, 1.f)))
                {
                    if (entry.is_directory())
                    {
                        state.currentDirectory /= path.filename();
                        state.selectedFilePath = "";
                    }
                    else
                    {
                        state.selectedFilePath = path;
                        AssetHandle handle = AssetManager::FindAssetHandle(path.string());
                        SceneHeirarchyPanel::SetSelectedEntity((Entity){});
                        InspectorPanel::SetSelectedAsset(handle);
                    }
                }

                if (ImGui::BeginDragDropSource())
                {
                    std::string relativePath = std::filesystem::relative(path, ProjectManager::GetAssetPath(CharmApp::GetProject()));
                    const char* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("Content Browser Item", itemPath, (strnlen(itemPath, 1024) + 1) * sizeof(char));
                    ImGui::EndDragDropSource();
                }
                ImGui::PopStyleColor(3);

                ImGui::TextWrapped("%s", filename.c_str());
                ImGui::NextColumn();
                ImGui::PopID();
            }

            ImGui::Columns(1);

            ImGui::End();
        }

        std::filesystem::path& GetSelectedFilePath() { return state.selectedFilePath; }
    }
}
