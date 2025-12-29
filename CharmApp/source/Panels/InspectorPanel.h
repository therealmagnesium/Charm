#pragma once
#include <Core/AssetManager.h>
#include <Core/Asset.h>

using namespace Charm::Core;

namespace CharmApp
{
    struct InspectorState
    {
        AssetHandle selectedAssetHandle = 0;
        Asset* selectedAsset = NULL;
        bool shouldDisplay = true;
    };

    namespace InspectorPanel
    {
        void Display();
        void Toggle();
        bool ShouldDisplay();
        void SetSelectedAsset(AssetHandle handle);
    }
}
