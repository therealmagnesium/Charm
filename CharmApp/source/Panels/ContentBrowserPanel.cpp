#include "ContentBrowserPanel.h"
#include "SceneHeirarchyPanel.h"
#include "InspectorPanel.h"
#include "../CharmApp.h"

#include <imgui.h>
#include <imgui_stdlib.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    static ContentBrowserState state;

    namespace ContentBrowserPanel
    {
        void Init()
        {
            state.currentDirectory = ProjectManager::GetAssetPath(CharmApp::GetProject());
            state.iconFile = Textures::Load("assets/textures/icon_file.png");
            state.iconFolder = Textures::Load("assets/textures/icon_folder.png");
            state.homeDirectory = Utils::GetHomeDirectory();
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

            const Project& project = CharmApp::GetProject();

            if (state.currentDirectory != ProjectManager::GetAssetPath(CharmApp::GetProject()))
                if (ImGui::Button("Back"))
                    state.currentDirectory = state.currentDirectory.parent_path();

            if (SceneHeirarchyPanel::GetSelectedEntity())
            {
                state.selectedFilePath.clear();
                InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
            }

            if (Input::IsMouseClicked(MOUSE_BUTTON_LEFT) && ImGui::IsWindowHovered())
                InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);

            const float cellSize = state.thumbnailSize + state.padding;
            const float panelWidth = ImGui::GetContentRegionAvail().x;
            u32 columnCount = (u32)(panelWidth / cellSize);
            if (columnCount < 1)
                columnCount = 1;

            ImGui::Columns(columnCount, NULL, false);

            auto directoryIterator = std::filesystem::directory_iterator(state.currentDirectory);
            for (auto& entry : directoryIterator)
            {
                const std::filesystem::path entryPath = entry.path();
                ImGui::PushID(entryPath.c_str());

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
                    if (!entry.is_directory())
                    {
                        SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                        state.selectedFilePath = entryPath;

                        AssetHandle handle = AssetManager::FindAssetHandle(entryPath);
                        InspectorPanel::SetSelectedAsset(handle);
                    }
                    else
                    {
                        InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                        state.selectedFilePath.clear();
                    }
                }

                if (ImGui::BeginDragDropSource())
                {
                    std::string relativePath = std::filesystem::relative(entryPath, ProjectManager::GetAssetPath(CharmApp::GetProject()));
                    const char* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("Content Browser Item", itemPath, (strnlen(itemPath, 1024) + 1) * sizeof(char));
                    ImGui::EndDragDropSource();
                }

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && entry.is_directory())
                {
                    state.currentDirectory /= entryPath.filename();
                    state.selectedFilePath.clear();
                }

                if (ImGui::BeginPopupContextItem("Asset Options Popup"))
                {
                    state.selectedFilePath = entryPath;

                    if (ImGui::MenuItem("Rename"))
                    {
                        const char* fileName = state.selectedFilePath.filename().c_str();
                        strncpy(state.rename.fileName, fileName, strlen(fileName) + 1);
                        state.rename.isActive = true;
                        state.rename.path = state.selectedFilePath;
                    }

                    if (ImGui::MenuItem("Delete"))
                    {
                        INFO("Deleting file %s...", entryPath.c_str());
                        if (AssetManager::IsAssetRegistered(entryPath))
                        {
                            AssetHandle handle = AssetManager::FindAssetHandle(entryPath);
                            AssetManager::Remove(handle);
                            InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                        }

                        std::filesystem::remove(entryPath);
                        InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                        state.selectedFilePath.clear();
                    }
                    ImGui::EndPopup();
                }

                ImGui::PopStyleColor(3);

                if (state.rename.isActive && entryPath == state.rename.path)
                {
                    const bool prevInputCapture = Input::GetCapture();
                    Input::Capture(true);
                    if (Input::IsKeyPressed(KEY_ESCAPE))
                    {
                        state.rename.isActive = false;
                        memset(state.rename.fileName, '\0', sizeof(state.rename.fileName));
                        state.rename.path.clear();
                    }
                    Input::Capture(prevInputCapture);

                    if (ImGui::InputText("##Filename", state.rename.fileName, LEN(state.rename.fileName) * sizeof(char), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        AssetHandle handle = AssetHandle_Invalid;
                        AssetType type = AssetType::Invalid;
                        if (AssetManager::IsAssetRegistered(entryPath))
                        {
                            handle = AssetManager::FindAssetHandle(entryPath);
                            type = AssetManager::GetAssetType(handle);
                            AssetManager::Remove(handle);
                        }

                        const std::filesystem::path newPath = entryPath.parent_path() / state.rename.fileName;
                        std::filesystem::rename(entryPath, newPath);

                        if (handle != AssetHandle_Invalid && type != AssetType::Invalid)
                            AssetManager::Import(newPath.c_str(), type, handle);

                        state.rename.isActive = false;
                        memset(state.rename.fileName, '\0', sizeof(state.rename.fileName));
                        state.rename.path.clear();
                    }
                }
                else
                    ImGui::TextUnformatted(entryPath.filename().c_str());

                ImGui::NextColumn();
                ImGui::PopID();
            }

            if (ImGui::BeginPopupContextWindow("Create Asset Popup", ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonRight))
            {
                if (ImGui::BeginMenu("Create"))
                {
                    ImGui::SeparatorText("System");
                    if (ImGui::MenuItem("Folder"))
                    {
                        const char* newFolderName = "New Folder";
                        const std::filesystem::path folderPath = ProjectManager::GetAssetFileSystemPath(newFolderName, project);
                        const std::filesystem::path projectPath = ProjectManager::GetAssetFileSystemPath(folderPath, project);
                        try
                        {
                            if (std::filesystem::create_directory(projectPath))
                                INFO("Folder \"%s\", was created successfully!", newFolderName);
                            else
                                ERROR("Folder \"%s\" could not be created either because it already exists, or Charm does not have permission", newFolderName);
                        }
                        catch (const std::filesystem::filesystem_error& e)
                        {
                            ERROR("Failed to create folder - %s", e.what());
                        }
                    }

                    ImGui::SeparatorText("Assets");
                    if (ImGui::MenuItem("Animation"))
                    {
                        FileDialogFilter filter;
                        filter.name = "Animation";
                        filter.specification = "anim";

                        FileDialogs::SetDefaultPath(state.currentDirectory);
                        if (FileDialogs::Save(&filter, 1))
                        {
                            const std::filesystem::path path = FileDialogs::GetSelectedPath();
                            const std::filesystem::path projectPath = ProjectManager::GetAssetFileSystemPath(path, project);

                            Animations::Save(projectPath.string().c_str(), Animation_Null);
                            AssetHandle handle = AssetManager::Import(projectPath.string().c_str(), AssetType::Animation);
                            SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                            InspectorPanel::SetSelectedAsset(handle);
                        }
                    }

                    if (ImGui::MenuItem("Animation Controller"))
                    {
                        FileDialogFilter filter;
                        filter.name = "Animation Controller";
                        filter.specification = "ac";

                        FileDialogs::SetDefaultPath(state.currentDirectory);
                        if (FileDialogs::Save(&filter, 1))
                        {
                            const std::filesystem::path path = FileDialogs::GetSelectedPath();
                            const std::filesystem::path projectPath = ProjectManager::GetAssetFileSystemPath(path, project);

                            Animations::SaveController(projectPath.string().c_str(), AnimationController_Null);
                            AssetHandle handle = AssetManager::Import(projectPath.string().c_str(), AssetType::AnimationController);
                            SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                            InspectorPanel::SetSelectedAsset(handle);
                        }
                    }

                    ImGui::MenuItem("Native Script");

                    ImGui::EndMenu();
                }
                ImGui::EndPopup();
            }

            ImGui::Columns(1);
            ImGui::End();
        }

        const std::filesystem::path& GetSelectedFilePath() { return state.selectedFilePath; }
        void ClearSelectedFilePath() { state.selectedFilePath.clear(); }
    }
}
