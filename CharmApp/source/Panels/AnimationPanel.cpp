#include "AnimationPanel.h"
#include "SceneHeirarchyPanel.h"
#include "ContentBrowserPanel.h"
#include "TextureSlicerPanel.h"

#include <Core/AssetManager.h>
#include <Core/FileDialogs.h>
#include <Graphics/Texture.h>
#include <ECS/Components.h>
#include <Projects/Project.h>

#include <imgui.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;
using namespace Charm::Projects;

namespace CharmApp
{
    static AnimationPanelState state;

    namespace AnimationPanel
    {
        void DrawAnimationPanel();
        void DrawAnimationList(AnimationController* controller);
        void DrawAnimationFrames(AnimationController* controller);

        void Display()
        {
            if (state.shouldDisplay)
                DrawAnimationPanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        bool ShouldDisplay() { return state.shouldDisplay; }

        void DrawAnimationPanel()
        {
            ImGui::Begin("Animations", &state.shouldDisplay);

            Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();
            if (selectedEntity.IsHandleValid() && selectedEntity.HasComponent<Animator2DComponent>())
            {
                const auto& animator = selectedEntity.GetComponent<Animator2DComponent>();

                if (animator.controller != AssetHandle_Invalid)
                {
                    AnimationController* controller = AssetManager::GetAsset<AnimationController>(animator.controller);
                    DrawAnimationList(controller);
                    ImGui::SameLine();
                    DrawAnimationFrames(controller);
                }
                else
                    ImGui::Text("Ensure that your selected entity has a valid animation controller attached to the animator component");
            }
            else
                ImGui::Text("Select an entity with an animator component to begin");

            ImGui::End();
        }

        void DrawAnimationList(AnimationController* controller)
        {
            ImVec2 availableRegion = ImGui::GetContentRegionAvail();
            ImVec2 windowSize = ImVec2(availableRegion.x / 4.f, 0.f);

            ImGui::BeginChild("##Animation List", windowSize, ImGuiChildFlags_ResizeX);

            if (controller->animations.size() > 0)
            {
                for (u32 i = 0; i < controller->animations.size(); i++)
                {
                    const AssetHandle animHandle = controller->animations[i];
                    const char* name = AssetManager::GetAssetPath(animHandle).stem().c_str();
                    const bool isSelected = state.selectedAnimation == animHandle;

                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

                    if (ImGui::Selectable(name, isSelected))
                        state.selectedAnimation = animHandle;

                    ImGui::PopStyleVar();
                }
            }

            ImGui::EndChild();
        }

        void DrawAnimationFrames(AnimationController* controller)
        {
            const ImVec2 buttonSize = ImVec2(25.f, 25.f);

            ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, buttonSize.y * 0.5f);
            ImGui::BeginChild("##Animation Frames", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_MenuBar);

            if (ImGui::BeginMenuBar())
            {
                const Texture& iconFolder = ContentBrowserPanel::GetIconFolder();
                const auto& colors = ImGui::GetStyle().Colors;
                const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
                const auto& buttonActive = colors[ImGuiCol_ButtonActive];
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(V3_OPEN(buttonHovered), 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(V3_OPEN(buttonActive), 0.5f));

                if (ImGui::ImageButton("##Add Frames", iconFolder.id, buttonSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f)) && state.selectedAnimation != AssetHandle_Invalid)
                {
                    FileDialogFilter filter;
                    filter.name = "Texture";
                    filter.specification = "png,jpg,jpeg";

                    if (FileDialogs::Open(&filter, 1))
                    {
                        const Project& project = ProjectManager::GetActive();
                        const std::filesystem::path relativePath = ProjectManager::GetAssetRelativePath(FileDialogs::GetSelectedPath(), project);
                        const AssetHandle handle = AssetManager::IsAssetRegistered(relativePath) ? AssetManager::FindAssetHandle(relativePath) : AssetManager::Import(relativePath, AssetType::Texture);

                        TextureSlicerPanel::SetSpriteSheet(handle);
                        TextureSlicerPanel::SetTargetAnimation(state.selectedAnimation);
                        if (!TextureSlicerPanel::ShouldDisplay())
                            TextureSlicerPanel::Toggle();
                    }
                }

                ImGui::PopStyleColor(3);
                ImGui::EndMenuBar();
            }
            ImGui::PopStyleVar();

            ImGui::Text("Frames will go here");
            ImGui::EndChild();
        }
    }
}
