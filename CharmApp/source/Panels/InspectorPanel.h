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
    };

    namespace InspectorPanel
    {
        void Display();

        void SetSelectedAsset(AssetHandle handle);
    }
}
