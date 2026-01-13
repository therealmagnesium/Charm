#include "AnimationPanel.h"
#include "SceneHeirarchyPanel.h"
#include "ContentBrowserPanel.h"
#include "TextureSlicerPanel.h"
#include "ToolbarPanel.h"

#include <Core/AssetManager.h>
#include <Core/FileDialogs.h>
#include <Core/Utils.h>
#include <Graphics/Texture.h>
#include <ECS/Components.h>
#include <Projects/Project.h>
#include <UI/UI.h>

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

        void Init()
        {
            state.icons[(u32)AnimationIcon::StepForward] = Textures::Load("assets/textures/icon_frame_step_forward.png");
            state.icons[(u32)AnimationIcon::StepBackward] = Textures::Load("assets/textures/icon_frame_step_backward.png");
            state.icons[(u32)AnimationIcon::StepFront] = Textures::Load("assets/textures/icon_frame_step_front.png");
            state.icons[(u32)AnimationIcon::StepBack] = Textures::Load("assets/textures/icon_frame_step_back.png");
        }

        void Shutdown()
        {
            for (Texture& texture : state.icons)
                Textures::Unload(texture);
        }

        void Display()
        {
            const Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();

            if (state.shouldPreviewAnimation && state.trackedEntity != selectedEntity)
            {
                state.shouldPreviewAnimation = false;
                state.selectedAnimation = AssetHandle_Invalid;
                state.trackedEntity = selectedEntity;
            }
            else if (!state.shouldPreviewAnimation)
            {
                state.trackedEntity = selectedEntity;
            }

            const bool isAnimationSelected = AssetManager::IsHandleValid(state.selectedAnimation);
            const bool isSpriteSheetValid = AssetManager::IsHandleValid(state.spriteSheet);

            if (state.shouldPreviewAnimation && isAnimationSelected)
            {
                if (state.activeCrop != NULL && isSpriteSheetValid)
                {
                    Animation* animation = AssetManager::GetAsset<Animation>(state.selectedAnimation);
                    Texture* spriteSheet = AssetManager::GetAsset<Texture>(state.spriteSheet);
                    Animations::Update(*animation);
                    Animations::Apply(*animation, *state.activeCrop, *spriteSheet);
                }
            }

            if (state.shouldDisplay)
                DrawAnimationPanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        bool ShouldDisplay() { return state.shouldDisplay; }

        void DrawAnimationPanel()
        {
            ImGui::Begin("Animation", &state.shouldDisplay);

            Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();
            if (selectedEntity.IsHandleValid() && selectedEntity.HasComponent<Animator2DComponent>())
            {
                if (selectedEntity.HasComponent<SpriteRendererComponent>())
                {
                    auto& spriteRenderer = selectedEntity.GetComponent<SpriteRendererComponent>();
                    state.spriteSheet = spriteRenderer.sprite;
                    state.activeCrop = &spriteRenderer.crop;
                }

                const auto& animator = selectedEntity.GetComponent<Animator2DComponent>();

                if (AssetManager::IsHandleValid(animator.controller))
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
            const ImVec2 fullAvailRegion = ImGui::GetContentRegionAvail();
            const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            const ImVec2 windowSize = ImVec2(fullAvailRegion.x / 4.f, 0.f);

            if (!AssetManager::IsHandleValid(state.selectedAnimation) && controller->animations.size() > 0)
                state.selectedAnimation = controller->animations[0];

            s32 animIndexToRemove = -1;
            ImGui::BeginChild("##Animation List", windowSize, ImGuiChildFlags_ResizeX);

            if (ImGui::Button("Add Animation", ImVec2(-1.f, lineHeight)))
            {
                FileDialogFilter filter;
                filter.name = "Animation";
                filter.specification = "anim";

                if (FileDialogs::Open(&filter, 1))
                {
                    const Project& project = ProjectManager::GetActive();
                    const std::filesystem::path path = FileDialogs::GetSelectedPath();
                    const std::filesystem::path relativePath = std::filesystem::proximate(path, ProjectManager::GetAssetPath(project));
                    const AssetHandle handle = AssetManager::FindAssetHandle(relativePath);

                    if (AssetManager::IsHandleValid(handle))
                        controller->animations.emplace_back(handle);
                }
            }

            if (controller->animations.size() > 0)
            {
                for (u32 i = 0; i < controller->animations.size(); i++)
                {
                    const AssetHandle animHandle = controller->animations[i];
                    const char* name = AssetManager::GetAssetPath(animHandle).stem().c_str();
                    const bool isSelected = state.selectedAnimation == animHandle;

                    ImGui::PushID(animHandle);
                    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.5f, 0.5f));

                    if (ImGui::Selectable(name, isSelected, ImGuiSelectableFlags_AllowOverlap))
                        state.selectedAnimation = animHandle;

                    ImGui::PopStyleVar();

                    const ImVec2 availButtonRegion = ImGui::GetContentRegionAvail();
                    ImGui::SameLine(availButtonRegion.x - lineHeight * 1.5f);

                    if (ImGui::Button("-", ImVec2(lineHeight, lineHeight)))
                        animIndexToRemove = i;

                    ImGui::PopID();
                }
            }

            if (animIndexToRemove > -1)
            {
                controller->animations.erase(controller->animations.begin() + animIndexToRemove);
                state.selectedAnimation = AssetHandle_Invalid;
            }

            ImGui::EndChild();
        }

        void DrawAnimationFrames(AnimationController* controller)
        {
            const ImVec2 fullAvailRegion = ImGui::GetContentRegionAvail();
            const ImVec2 buttonSize = ImVec2(25.f, 25.f);
            const bool isAnimationSelected = AssetManager::IsHandleValid(state.selectedAnimation);

            ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, buttonSize.y * 0.5f);
            ImGui::BeginChild("##Animation Frames", ImVec2(0.f, 0.f), ImGuiChildFlags_None, ImGuiWindowFlags_MenuBar);

            if (ImGui::BeginMenuBar())
            {
                ImGui::BeginDisabled(!isAnimationSelected);
                Animation* selectedAnimation = AssetManager::GetAsset<Animation>(state.selectedAnimation);
                s32* speed = isAnimationSelected ? (s32*)&selectedAnimation->speed : (s32*)&Animation_Null.speed;
                ImGui::SetNextItemWidth(50.f);
                ImGui::InputInt("##Speed", speed, 0, 0);
                ImGui::EndDisabled();

                const Texture& iconFolder = ContentBrowserPanel::GetIconFolder();
                const Texture& iconPlay = ToolbarPanel::GetIcon(ToolbarIcon::Play);
                const Texture& iconStop = ToolbarPanel::GetIcon(ToolbarIcon::Stop);

                const auto& colors = ImGui::GetStyle().Colors;
                const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
                const auto& buttonActive = colors[ImGuiCol_ButtonActive];
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(V3_OPEN(buttonHovered), 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(V3_OPEN(buttonActive), 0.5f));

                const ImTextureRef frameStepForwardRef = state.icons[(u32)AnimationIcon::StepForward].id;
                const ImTextureRef frameStepBackwardRef = state.icons[(u32)AnimationIcon::StepBackward].id;
                const ImTextureRef frameStepFrontRef = state.icons[(u32)AnimationIcon::StepFront].id;
                const ImTextureRef frameStepBackRef = state.icons[(u32)AnimationIcon::StepBack].id;
                const ImTextureRef previewRef = !state.shouldPreviewAnimation ? iconPlay.id : iconStop.id;

                const ImVec2 uv0 = ImVec2(0.f, 0.f);
                const ImVec2 uv1 = ImVec2(1.f, 1.f);

                if (ImGui::ImageButton("##Frame Step Back", frameStepBackRef, buttonSize, uv0, uv1) && isAnimationSelected)
                {
                    Animation* animation = AssetManager::GetAsset<Animation>(state.selectedAnimation);
                    if (animation->frames.size() > 0)
                    {
                        animation->currentFrame = 0;
                        *state.activeCrop = animation->frames[animation->currentFrame];
                    }
                }

                if (ImGui::ImageButton("##Frame Step Backward", frameStepBackwardRef, buttonSize, uv0, uv1) && isAnimationSelected)
                {
                    Animation* animation = AssetManager::GetAsset<Animation>(state.selectedAnimation);

                    if (animation->currentFrame > 0)
                        animation->currentFrame--;

                    *state.activeCrop = animation->frames[animation->currentFrame];
                }

                if (ImGui::ImageButton("##Play Animation", previewRef, buttonSize, uv0, uv1) && isAnimationSelected)
                {
                    state.shouldPreviewAnimation = !state.shouldPreviewAnimation;

                    Animation* animation = AssetManager::GetAsset<Animation>(state.selectedAnimation);
                    if (!animation->shouldLoop && !state.shouldPreviewAnimation)
                    {
                        Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();
                        auto& spriteRenderer = selectedEntity.GetComponent<SpriteRendererComponent>();
                        spriteRenderer.crop = animation->frames[0];
                        Animations::Reset(*animation);
                    }
                }

                if (ImGui::ImageButton("##Frame Step Forward", frameStepForwardRef, buttonSize, uv0, uv1) && isAnimationSelected)
                {
                    Animation* animation = AssetManager::GetAsset<Animation>(state.selectedAnimation);

                    if (animation->currentFrame < animation->frames.size() - 1)
                        animation->currentFrame++;

                    *state.activeCrop = animation->frames[animation->currentFrame];
                }

                if (ImGui::ImageButton("##Frame Step Front", frameStepFrontRef, buttonSize, uv0, uv1) && isAnimationSelected)
                {
                    Animation* animation = AssetManager::GetAsset<Animation>(state.selectedAnimation);
                    if (animation->frames.size() > 0)
                    {
                        const u32 lastFrameIndex = animation->frames.size() - 1;
                        animation->currentFrame = lastFrameIndex;
                        *state.activeCrop = animation->frames[animation->currentFrame];
                    }
                }

                if (ImGui::ImageButton("##Add Frames", iconFolder.id, buttonSize, uv0, uv1) && isAnimationSelected)
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

            if (isAnimationSelected)
            {
                Animation* selectedAnimation = AssetManager::GetAsset<Animation>(state.selectedAnimation);
                if (ImGui::BeginTable("Animation Frames Table", selectedAnimation->frames.size(), ImGuiTableFlags_None))
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    Texture* spriteSheet = AssetManager::GetAsset<Texture>(state.spriteSheet);
                    for (u32 i = 0; i < selectedAnimation->frames.size(); i++)
                    {
                        const std::string stringID = "##Frame " + std::to_string(i);
                        const Rectangle& frame = selectedAnimation->frames[i];
                        const ImVec2 uv0 = ImVec2(frame.x / spriteSheet->width, frame.y / spriteSheet->height);
                        const ImVec2 uv1 = ImVec2((frame.x + frame.width) / spriteSheet->width, (frame.y + frame.height) / spriteSheet->height);
                        ImGui::ImageButton(stringID.c_str(), spriteSheet->id, ImVec2(64.f, 64.f), uv0, uv1);
                        ImGui::TableNextColumn();
                    }

                    ImGui::EndTable();
                }
            }
            ImGui::EndChild();
        }
    }
}
