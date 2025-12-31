#pragma once
#include <Core/Base.h>
#include <Core/Asset.h>

using namespace Charm;

namespace CharmApp
{
    struct TextureSlicerPanelState
    {
        Core::AssetHandle texture = Core::AssetHandle_Invalid;
        u32 sliceWidth = 0;
        u32 sliceHeight = 0;
        bool shouldDisplay = false;
    };

    namespace TextureSlicerPanel
    {
        void Display();
        void Toggle();

        bool ShouldDisplay();
        void SetSpriteSheet(Core::AssetHandle spriteSheetHandle);
        void SetSliceWidth(u32 sliceWidth);
        void SetSliceHeight(u32 sliceHeight);
    }
}
