#pragma once
#include <Core/Base.h>
#include <Core/Asset.h>
#include <vector>

using namespace Charm;

namespace CharmApp
{
    struct FrameSelection
    {
        u32 row = 0;
        u32 column = 0;
        u32 frameIndex = 0;
    };

    struct TextureSlicerPanelState
    {
        std::vector<FrameSelection> selectedFrames;
        Core::AssetHandle texture = Core::AssetHandle_Invalid;
        Core::AssetHandle targetAnimation = Core::AssetHandle_Invalid;
        u32 sliceWidth = 0;
        u32 sliceHeight = 0;
        u32 columnCount = 0;
        u32 rowCount = 0;
        bool shouldDisplay = false;
    };

    namespace TextureSlicerPanel
    {
        void Display();
        void Toggle();

        bool ShouldDisplay();
        void SetSpriteSheet(Core::AssetHandle spriteSheetHandle);
        void SetTargetAnimation(Core::AssetHandle targetAnimationHandle);
        void SetSliceWidth(u32 sliceWidth);
        void SetSliceHeight(u32 sliceHeight);
        void ClearSelection();
    }
}
