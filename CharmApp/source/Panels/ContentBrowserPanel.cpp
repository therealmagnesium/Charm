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
    static bool shouldLog = true;

    namespace ContentBrowserPanel
    {
        void DrawBrowserAssets(const std::filesystem::directory_entry& entry);
        void DrawCreateAssetPopup();

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

            const ImGuiStyle& style = ImGui::GetStyle();
            const float panelWidth = ImGui::GetContentRegionAvail().x;
            const ImVec2 backButtonLabelSize = ImGui::CalcTextSize("Back", NULL, true);
            const ImVec2 backButtonSize = ImVec2(backButtonLabelSize.x + style.FramePadding.x * 2.f, backButtonLabelSize.y + style.FramePadding.y * 2.f);

            ImGui::PushID("View Settings");
            ImGui::Columns(3);
            ImGui::SetColumnWidth(0, 140.f);
            ImGui::SetColumnWidth(1, panelWidth - backButtonSize.x - 140.f);
            ImGui::Text("Thumbnail size");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.f);
            ImGui::SliderFloat("##Thumbnail size", &state.thumbnailSize, 64.f, 300.f);
            ImGui::NextColumn();

            if (state.currentDirectory != ProjectManager::GetAssetPath(CharmApp::GetProject()))
                if (ImGui::Button("Back"))
                    state.currentDirectory = state.currentDirectory.parent_path();

            ImGui::Columns(1);
            ImGui::PopID();

            ImGui::Separator();

            if (SceneHeirarchyPanel::GetSelectedEntity())
            {
                state.selectedFilePath.clear();
                InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
            }

            /*
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
                    {
                        state.selectedFilePath.clear();
                        InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                    }*/

            const float cellSize = state.thumbnailSize + state.padding;
            state.columnCount = (u32)(panelWidth / cellSize);
            state.padding = state.thumbnailSize * 1.515625f;
            if (state.columnCount < 1)
                state.columnCount = 1;

            if (ImGui::BeginTable("Content Browser Asset Table", state.columnCount, ImGuiTableFlags_None))
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                auto directoryIterator = std::filesystem::directory_iterator(state.currentDirectory);
                for (const auto& entry : directoryIterator)
                    DrawBrowserAssets(entry);

                shouldLog = false;

                if (ImGui::BeginPopupContextWindow("Create Asset Popup", ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonRight))
                    DrawCreateAssetPopup();

                ImGui::EndTable();
            }

            ImGui::End();
        }

        const std::filesystem::path& GetSelectedFilePath() { return state.selectedFilePath; }
        void ClearSelectedFilePath() { state.selectedFilePath.clear(); }
        void SetCurrentDirectory(const std::filesystem::path& path) { state.currentDirectory = path; }

        void DrawBrowserAssets(const std::filesystem::directory_entry& entry)
        {
            const float padding = state.thumbnailSize * 0.5f;
            const Project& project = ProjectManager::GetActive();

            const std::filesystem::path entryPath = entry.path();
            const std::filesystem::path relativeAssetPath = ProjectManager::GetAssetRelativePath(entryPath, project);
            ImGui::PushID(entryPath.c_str());

            ImTextureID icon = (entry.is_directory()) ? state.iconFolder.id : state.iconFile.id;
            ImVec2 buttonSize = ImVec2(state.thumbnailSize, state.thumbnailSize);

            // Display the folder/file icon and handle what happens when it's clicked
            const auto& colors = ImGui::GetStyle().Colors;
            const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
            const auto& buttonActive = colors[ImGuiCol_ButtonActive];
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(V3_OPEN(buttonHovered), 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(V3_OPEN(buttonActive), 0.5f));

            const float iconIndent = (state.thumbnailSize + state.padding - state.thumbnailSize) * 0.5f;
            ImGui::Indent(iconIndent);
            if (ImGui::ImageButton("##Icon", icon, buttonSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f)))
            {
                if (!entry.is_directory())
                {
                    SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                    state.selectedFilePath = relativeAssetPath;

                    const AssetHandle handle = AssetManager::FindAssetHandle(relativeAssetPath);
                    InspectorPanel::SetSelectedAsset(handle);
                }
                else
                {
                    InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                    state.selectedFilePath.clear();
                }
            }
            ImGui::Unindent(iconIndent);

            // Allow the icons for each asset to be draggable onto other ImGui windows and widgets
            if (ImGui::BeginDragDropSource())
            {
                const char* itemPath = relativeAssetPath.c_str();
                ImGui::SetDragDropPayload("Content Browser Item", itemPath, (strnlen(itemPath, 1024) + 1) * sizeof(char));
                ImGui::EndDragDropSource();
            }

            // Handle double clicking on folders to update the current searched directory
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && entry.is_directory())
            {
                state.currentDirectory /= entryPath.filename();
                state.selectedFilePath.clear();
                shouldLog = true;
            }

            // Allow right clicking on the icons to bring up a popup to display options for the file on disk
            if (ImGui::BeginPopupContextItem("Asset Options Popup"))
            {
                state.selectedFilePath = ProjectManager::GetAssetRelativePath(entryPath, project);

                // If the user decides to rename the file/folder, setup the rename state for the Content Browser Panel
                if (ImGui::MenuItem("Rename"))
                {
                    const char* fileName = state.selectedFilePath.filename().c_str();
                    strncpy(state.rename.fileName, fileName, strlen(fileName) + 1);
                    state.rename.isActive = true;
                    state.rename.path = state.selectedFilePath;
                }

                // If the user decides to delete a file/folder, delete the file on disk and remove the asset from the registry if it's already registered
                if (ImGui::MenuItem("Delete"))
                {
                    INFO("Deleting file %s...", entryPath.c_str());
                    if (!entry.is_directory() && AssetManager::IsAssetRegistered(entryPath))
                    {
                        const AssetHandle handle = AssetManager::FindAssetHandle(relativeAssetPath);
                        AssetManager::Remove(handle);
                        InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                    }

                    std::filesystem::remove(entryPath);
                    InspectorPanel::SetSelectedAsset(AssetHandle_Invalid);
                    state.selectedFilePath.clear();
                }
                ImGui::EndPopup();
            }

            // Check for the user renaming a file on disk
            if (state.rename.isActive && relativeAssetPath == state.rename.path)
            {
                // Cancel renaming the file if the user presses 'Escape'
                const bool prevInputCapture = Input::GetCapture();
                Input::Capture(true);
                if (Input::IsKeyPressed(KEY_ESCAPE))
                {
                    state.rename.isActive = false;
                    memset(state.rename.fileName, '\0', sizeof(state.rename.fileName));
                    state.rename.path.clear();
                }
                Input::Capture(prevInputCapture);

                // If the user submitted the updated file name, rename the file, and then update the asset registry with the updated file name
                const float textWidth = ImGui::CalcTextSize(state.rename.fileName).x;
                const float indent = (state.thumbnailSize + state.padding - textWidth) * 0.3f;
                ImGui::Indent(indent);
                if (ImGui::InputText("##Filename", state.rename.fileName, LEN(state.rename.fileName) * sizeof(char), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ElideLeft))
                {
                    AssetHandle handle = AssetHandle_Invalid;
                    AssetType type = AssetType::Invalid;
                    if (AssetManager::IsAssetRegistered(relativeAssetPath))
                    {
                        handle = AssetManager::FindAssetHandle(relativeAssetPath);
                        type = AssetManager::GetAssetType(handle);
                        AssetManager::Remove(handle);
                    }

                    const std::filesystem::path newPath = ProjectManager::GetAssetFileSystemPath(state.rename.path.parent_path() / state.rename.fileName, project);
                    std::filesystem::rename(entryPath, newPath);

                    if (handle != AssetHandle_Invalid && type != AssetType::Invalid)
                        AssetManager::Import(newPath, type, handle);

                    state.rename.isActive = false;
                    memset(state.rename.fileName, '\0', sizeof(state.rename.fileName));
                    state.rename.path.clear();
                }
                ImGui::Unindent(indent);
            }
            else
            {
                // If the user is not renaming a file, just display the file name under the folder/file icon
                const std::filesystem::path fileNameAsPath = entryPath.stem();
                const char* fileName = fileNameAsPath.c_str();
                const float textWidth = ImGui::CalcTextSize(fileName).x;
                const float indent = (state.thumbnailSize + state.padding - textWidth) * 0.5f;

                ImGui::Indent(indent);
                ImGui::TextUnformatted(fileName);
                ImGui::Unindent(indent);

                // WARN("%s", fileName.c_str());
            }

            ImGui::PopStyleColor(3);

            ImGui::TableNextColumn();
            ImGui::PopID();
        }

        void DrawCreateAssetPopup()
        {
            const Project& project = CharmApp::GetProject();

            if (ImGui::BeginMenu("Create"))
            {
                ImGui::SeparatorText("System");

                // Try to create a folder on disk, if it fails for whatever reason, an error message will be printed to the console
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

                // Bring up a file dialog to allow the user to create and save a new animation file on disk
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
                        AssetHandle handle = AssetManager::Import(projectPath, AssetType::Animation);
                        SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                        InspectorPanel::SetSelectedAsset(handle);
                    }
                }

                // Bring up a file dialog to allow the user to create and save a new animation controller file on disk
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
                        AssetHandle handle = AssetManager::Import(projectPath, AssetType::AnimationController);
                        SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                        InspectorPanel::SetSelectedAsset(handle);
                    }
                }

                ImGui::MenuItem("Native Script");
                ImGui::EndMenu();
            }
            ImGui::EndPopup();
        }
    }
}
