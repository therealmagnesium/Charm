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
using namespace Charm::ECS;

namespace CharmApp
{
    static TextureSlicerPanelState state;

    namespace TextureSlicerPanel
    {
        void DrawTextureSlicerPanel();
        void DrawSliceControls();
        void DrawCanvas();

        void Display()
        {
            if (!AssetManager::IsHandleValid(state.texture))
                return;

            if (!AssetManager::IsHandleValid(state.targetAnimation))
                return;

            if (state.shouldDisplay)
                DrawTextureSlicerPanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        bool ShouldDisplay() { return state.shouldDisplay; }
        void SetSpriteSheet(AssetHandle spriteSheetHandle) { state.texture = spriteSheetHandle; }
        void SetSliceWidth(u32 sliceWidth) { state.sliceWidth = sliceWidth; }
        void SetSliceHeight(u32 sliceHeight) { state.sliceHeight = sliceHeight; }
        void ClearSelection() { state.selectedFrames.clear(); }
        void SetTargetAnimation(Core::AssetHandle targetAnimationHandle)
        {
            state.targetAnimation = targetAnimationHandle;
            state.selectedFrames.clear();
        }

        void DrawTextureSlicerPanel()
        {
            ImGui::Begin("Texture Slicer", &state.shouldDisplay, ImGuiWindowFlags_NoDocking);

            DrawSliceControls();
            DrawCanvas();

            ImGui::End();
        }

        void DrawSliceControls()
        {
            const float columnWidth = 120.f;

            UI::DrawIntInputControl("Column Count", (s32*)&state.columnCount, 0, 0, columnWidth);
            UI::DrawIntInputControl("Row Count", (s32*)&state.rowCount, 0, 0, columnWidth);
            UI::DrawIntInputControl("Slice Width", (s32*)&state.sliceWidth, 0, 0, columnWidth);
            UI::DrawIntInputControl("Slice Height", (s32*)&state.sliceHeight, 0, 0, columnWidth);

            if (ImGui::Button("Apply"))
            {
                Animation* animation = AssetManager::GetAsset<Animation>(state.targetAnimation);
                animation->frameCount = state.selectedFrames.size();
                animation->frames.resize(animation->frameCount);

                for (u32 i = 0; i < animation->frameCount; i++)
                {
                    FrameSelection& frameSelection = state.selectedFrames[i];
                    const float x = frameSelection.column * state.sliceWidth;
                    const float y = frameSelection.row * state.sliceHeight;
                    const float width = state.sliceWidth;
                    const float height = state.sliceHeight;
                    const Rectangle frame = (Rectangle){x, y, width, height};

                    animation->frames[frameSelection.frameIndex] = frame;
                }

                Toggle();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
                ClearSelection();

            /*
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, columnWidth);
                    ImGui::Text("Sprite Sheet");
                    ImGui::NextColumn();

                    std::string placeholder = "Select a sprite sheet";
                    if (AssetManager::IsHandleValid(state.texture))
                        placeholder = AssetManager::GetAssetPath(state.texture).stem().string();

                    const float applyButtonWidth = ImGui::CalcTextSize("Apply").x;
                    const float clearButtonWidth = ImGui::CalcTextSize("Clear").x;
                    ImGui::SetNextItemWidth(-1.5f * (applyButtonWidth + clearButtonWidth));
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
                                state.sliceWidth = 32;
                                state.sliceHeight = 32;
                                state.columnCount = texture->width / state.sliceWidth;
                                state.rowCount = texture->height / state.sliceHeight;
                            }
                        }

                        ImGui::EndCombo();
                    }

                    ImGui::Columns(1);*/
        }

        void DrawCanvas()
        {
            Texture* spriteSheet = AssetManager::GetAsset<Texture>(state.texture);

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

            // Calculate the scale factor between texture pixels and screen pixels
            const float scaleX = aspectSize.x / (float)spriteSheet->width;
            const float scaleY = aspectSize.y / (float)spriteSheet->height;

            // Calculate scaled slice dimensions
            const float scaledSliceWidth = state.sliceWidth * scaleX;
            const float scaledSliceHeight = state.sliceHeight * scaleY;

            // Create an invisible button over the entire canvas for input handling
            ImGui::SetCursorScreenPos(canvasTopLeft);
            ImGui::InvisibleButton("##Canvas", aspectSize, ImGuiButtonFlags_MouseButtonLeft);
            const bool isCanvasHovered = ImGui::IsItemHovered();
            const bool isCanvasClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
            if (isCanvasClicked)
            {
                const ImVec2 mousePosition = ImGui::GetMousePos();
                const ImVec2 relativePos = ImVec2(mousePosition.x - canvasTopLeft.x, mousePosition.y - canvasTopLeft.y);

                // Calculate which frame was clicked
                const u32 column = (u32)(relativePos.x / scaledSliceWidth);
                const u32 row = (u32)(relativePos.y / scaledSliceHeight);

                // Ensure the click is within bounds
                if (column < state.columnCount && row < state.rowCount)
                {
                    // Check if this frame is already selected
                    auto it = std::find_if(state.selectedFrames.begin(), state.selectedFrames.end(),
                                           [row, column](const FrameSelection& fs) { return fs.row == row && fs.column == column; });

                    if (it != state.selectedFrames.end())
                    {
                        // Frame already selected - remove it and adjust indices
                        u32 removedIndex = it->frameIndex;
                        state.selectedFrames.erase(it);

                        // Adjust frame indices for frames that came after the removed one
                        for (auto& frame : state.selectedFrames)
                        {
                            if (frame.frameIndex > removedIndex)
                                frame.frameIndex--;
                        }
                    }
                    else
                    {
                        // Add new frame selection
                        FrameSelection selection;
                        selection.row = row;
                        selection.column = column;
                        selection.frameIndex = (u32)state.selectedFrames.size();
                        state.selectedFrames.emplace_back(selection);
                    }
                }
            }

            // Draw a grid of all the slices
            for (u32 row = 0; row < state.rowCount; row++)
            {
                for (u32 col = 0; col < state.columnCount; col++)
                {
                    ImVec2 topLeft;
                    topLeft.x = canvasTopLeft.x + col * scaledSliceWidth;
                    topLeft.y = canvasTopLeft.y + row * scaledSliceHeight;

                    ImVec2 bottomRight;
                    bottomRight.x = topLeft.x + scaledSliceWidth;
                    bottomRight.y = topLeft.y + scaledSliceHeight;

                    drawList->AddRect(topLeft, bottomRight, IM_COL32(50, 168, 82, 255));

                    // Check if this frame is selected
                    auto it = std::find_if(state.selectedFrames.begin(), state.selectedFrames.end(),
                                           [row, col](const FrameSelection& fs) { return fs.row == row && fs.column == col; });

                    if (it != state.selectedFrames.end())
                    {
                        // Draw semi-transparent overlay
                        drawList->AddRectFilled(topLeft, bottomRight, IM_COL32(0, 0, 0, 150));

                        // Draw frame number
                        char frameNumText[8];
                        snprintf(frameNumText, sizeof(frameNumText), "%u", it->frameIndex);

                        ImVec2 textSize = ImGui::CalcTextSize(frameNumText);
                        ImVec2 textPosition;
                        textPosition.x = topLeft.x + (scaledSliceWidth - textSize.x) * 0.5f;
                        textPosition.y = topLeft.y + (scaledSliceHeight - textSize.y) * 0.5f;

                        // Draw text with outline for better visibility
                        drawList->AddText(ImVec2(textPosition.x + 1, textPosition.y + 1), IM_COL32(0, 0, 0, 255), frameNumText);
                        drawList->AddText(textPosition, IM_COL32(255, 255, 255, 255), frameNumText);
                    }

                    // Highlight hovered frame
                    if (isCanvasHovered)
                    {
                        ImVec2 mousePos = ImGui::GetMousePos();
                        if (mousePos.x >= topLeft.x && mousePos.x <= bottomRight.x &&
                            mousePos.y >= topLeft.y && mousePos.y <= bottomRight.y)
                        {
                            drawList->AddRect(topLeft, bottomRight, IM_COL32(255, 255, 100, 255), 0.0f, 0, 2.0f);
                        }
                    }
                }
            }
        }
    }
}
