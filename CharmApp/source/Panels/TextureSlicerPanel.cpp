#include "TextureSlicerPanel.h"
#include "SceneHeirarchyPanel.h"

#include <Core/AssetManager.h>
#include <ECS/Components.h>
#include <Graphics/Animation.h>
#include <Graphics/Texture.h>
#include <UI/UI.h>

#include <imgui.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static TextureSlicerPanelState state;

    namespace TextureSlicerPanel
    {
        void DrawTextureSlicerPanel();
        void DrawSliceControls();

        void Display()
        {
            if (state.shouldDisplay)
                DrawTextureSlicerPanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        bool ShouldDisplay() { return state.shouldDisplay; }
        void SetSpriteSheet(AssetHandle spriteSheetHandle) { state.texture = spriteSheetHandle; }
        void SetSliceWidth(u32 sliceWidth) { state.sliceWidth = sliceWidth; }
        void SetSliceHeight(u32 sliceHeight) { state.sliceHeight = sliceHeight; }

        void DrawTextureSlicerPanel()
        {
            ImGui::Begin("Texture Slicer", &state.shouldDisplay, ImGuiWindowFlags_NoDocking);

            DrawSliceControls();

            if (!AssetManager::IsHandleValid(state.texture))
            {
                ImGui::End();
                return;
            }

            Texture* spriteSheet = AssetManager::GetAsset<Texture>(state.texture);

            // Display "slice" button
            ImGui::SameLine();
            const bool shouldSlice = ImGui::Button("Slice");
            ImGui::Columns(1);

            // Calculate canvas properties
            ImVec2 canvasTopLeft = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
            if (canvasSize.y < 50.0f) canvasSize.y = 50.0f;
            ImVec2 canvasBottomRight = ImVec2(canvasTopLeft.x + canvasSize.x, canvasTopLeft.y + canvasSize.y);

            // Draw the canvas
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(canvasTopLeft, canvasBottomRight, IM_COL32(50, 50, 50, 255));
            drawList->AddRect(canvasTopLeft, canvasBottomRight, IM_COL32(255, 255, 255, 255));

            // Draw sprite sheet over the canvas
            const float targetAspect = (float)spriteSheet->width / (float)spriteSheet->height;
            ImVec2 aspectSize = ImVec2(canvasSize.x, canvasSize.x / targetAspect);

            if (aspectSize.y > canvasSize.y)
            {
                aspectSize.y = canvasSize.y;
                aspectSize.x = canvasSize.y * targetAspect;
            }

            ImGui::SetCursorScreenPos(canvasTopLeft);
            ImGui::Image(spriteSheet->id, aspectSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));

            if (state.sliceWidth > 0 && state.sliceHeight > 0)
            {
                const u32 numCols = spriteSheet->width / state.sliceWidth;
                const u32 numRows = spriteSheet->height / state.sliceHeight;

                // Handle what happens when the "slice" button is clicked
                if (shouldSlice)
                {
                    spriteSheet->rowCount = numRows;
                    spriteSheet->columnCount = numCols;

                    Scene* context = SceneHeirarchyPanel::GetContext();
                    auto sprites = context->registry.view<SpriteRendererComponent>();
                    for (auto spriteID : sprites)
                    {
                        Entity entity = Entities::Create(spriteID, context);
                        auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                        // Skip if the texture we're slicing doesn't match the entity's texture reference handle
                        if (spriteRenderer.sprite != state.texture)
                            continue;

                        // Set the sprite renderer's crop size to be the size of each slice
                        spriteRenderer.crop.width = state.sliceWidth;
                        spriteRenderer.crop.height = state.sliceHeight;

                        // Check if the entity also has an animator component
                        bool hasAnimator = entity.HasComponent<Animator2DComponent>();
                        if (hasAnimator)
                        {
                            auto& animator = entity.GetComponent<Animator2DComponent>();
                            if (!AssetManager::IsHandleValid(animator.controller))
                                continue;

                            // For every animation in the animation controller, set the sprite sheet type
                            AnimationController* controller = AssetManager::GetAsset<AnimationController>(animator.controller);
                            for (u32 i = 0; i < controller->animations.size(); i++)
                            {
                                AssetHandle animHandle = controller->animations[i];
                                if (!AssetManager::IsHandleValid(animHandle))
                                    continue;

                                Animation* animation = AssetManager::GetAsset<Animation>(animHandle);
                                animation->spriteSheetType = (numCols > numRows) ? SpriteSheetAnimType::Horizontal : SpriteSheetAnimType::Vertical;
                            }
                        }
                    }
                }

                // Calculate the scale factor between texture pixels and screen pixels
                const float scaleX = aspectSize.x / (float)spriteSheet->width;
                const float scaleY = aspectSize.y / (float)spriteSheet->height;

                // Calculate scaled slice dimensions
                const float scaledSliceWidth = state.sliceWidth * scaleX;
                const float scaledSliceHeight = state.sliceHeight * scaleY;

                // Draw a grid of all the slices
                for (u32 row = 0; row < numRows; row++)
                {
                    for (u32 col = 0; col < numCols; col++)
                    {
                        ImVec2 topLeft;
                        topLeft.x = canvasTopLeft.x + col * scaledSliceWidth;
                        topLeft.y = canvasTopLeft.y + row * scaledSliceHeight;

                        ImVec2 bottomRight;
                        bottomRight.x = topLeft.x + scaledSliceWidth;
                        bottomRight.y = topLeft.y + scaledSliceHeight;

                        drawList->AddRect(topLeft, bottomRight, IM_COL32(50, 168, 82, 255));
                    }
                }
            }
            ImGui::End();
        }

        void DrawSliceControls()
        {
            const float columnWidth = 120.f;

            UI::DrawIntInputControl("Slice Width", (s32*)&state.sliceWidth, 0, 0, columnWidth);
            UI::DrawIntInputControl("Slice Height", (s32*)&state.sliceHeight, 0, 0, columnWidth);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("Sprite Sheet");
            ImGui::NextColumn();

            std::string placeholder = "Select a sprite sheet";
            if (AssetManager::IsHandleValid(state.texture))
                placeholder = AssetManager::GetAssetPath(state.texture).stem().string();

            const float sliceButtonWidth = ImGui::CalcTextSize("Slice").x;
            ImGui::SetNextItemWidth(-1.5f * sliceButtonWidth);
            if (ImGui::BeginCombo("##Sprite Sheet Selection", placeholder.c_str()))
            {
                if (ImGui::Selectable("None", !AssetManager::IsHandleValid(state.texture)))
                    state.texture = AssetHandle_Invalid;

                const std::vector<AssetHandle> textures = AssetManager::GetAllHandlesOfType(AssetType::Texture);
                for (AssetHandle handle : textures)
                {
                    Texture* texture = AssetManager::GetAsset<Texture>(handle);
                    if (texture->mode == TextureMode::Single)
                        continue;

                    const bool isSelected = (state.texture == handle);
                    const std::string stemName = AssetManager::GetAssetPath(handle).stem().string();
                    if (ImGui::Selectable(stemName.c_str(), isSelected))
                    {
                        state.texture = handle;
                        state.sliceWidth = 0;
                        state.sliceHeight = 0;

                        if (texture->rowCount > 1 || texture->columnCount > 1)
                        {
                            state.sliceWidth = texture->width / texture->columnCount;
                            state.sliceHeight = texture->height / texture->rowCount;
                        }
                    }
                }

                ImGui::EndCombo();
            }
        }
    }
}
