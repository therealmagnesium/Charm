#pragma once
#include <Core/Base.h>
#include <Core/Asset.h>
#include <Graphics/TilePalette.h>

using namespace Charm;

namespace CharmApp
{
    enum class TilePaletteMode : u32
    {
        InvalidEmpty = 0, // No tile palettes exist in the asset registry
        Invalid,          // At least one tile palette was found in the asset registry, but has not been assigned to the panel yet
        ValidNoTileset,   // A tile palette has been assigned to the panel, but it has no tileset to slice
        ValidUnsliced,    // A tile palette has been assigned to the panel, has a tileset assigned, but has not been sliced yet
        ValidSliced       // The tile palette panel is ready to paint sliced tiles from the tileset, and the user can now save the tilemap
    };

    struct TilePalettePanelState
    {
        Core::AssetHandle tilePalette = Core::AssetHandle_Invalid;
        TilePaletteMode mode = TilePaletteMode::InvalidEmpty;
        bool shouldDisplay = false;
    };

    namespace TilePalettePanel
    {
        void Display();
        void Toggle();

        bool ShouldDisplay();
    }
}
