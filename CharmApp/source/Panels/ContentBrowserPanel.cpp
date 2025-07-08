#include "ContentBrowserPanel.h"
#include <imgui.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    static ContentBrowserState state;
    static std::filesystem::path assetsDirectory = "assets";

    namespace ContentBrowserPanel
    {
        void Init()
        {
            state.currentDirectory = assetsDirectory;
            state.iconFile = Textures::Load("assets/textures/charm/file_icon.png");
            state.iconFolder = Textures::Load("assets/textures/charm/folder_icon.png");
        }

        void Shutdown()
        {
            Textures::Unload(state.iconFile);
            Textures::Unload(state.iconFolder);
        }

        void Display()
        {
            ImGui::Begin("Content Browser");

            if (state.currentDirectory != assetsDirectory)
            {
                if (ImGui::Button("Back"))
                    state.currentDirectory = state.currentDirectory.parent_path();
            }

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
                                       ImVec2(0.f, 1.f), ImVec2(1.f, 0.f)))
                {
                    if (entry.is_directory())
                        state.currentDirectory /= path.filename();
                }

                if (ImGui::BeginDragDropSource())
                {
                    const char* itemPath = path.c_str();
                    ImGui::SetDragDropPayload("Content Browser Item", itemPath, (strnlen(itemPath, 1024) + 1) * sizeof(char));
                    ImGui::EndDragDropSource();
                }
                ImGui::PopStyleColor(3);

                ImGui::TextWrapped("%s", filename.c_str());
                ImGui::NextColumn();
                ImGui::PopID();
            }

            ImGui::Columns(1);

            ImGui::DragFloat("Padding", &state.padding);
            ImGui::DragFloat("Thumbnail size", &state.thumbnailSize);
            ImGui::End();
        }
    }
}
