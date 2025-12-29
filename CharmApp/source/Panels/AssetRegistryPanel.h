#pragma once
#include <Core/Base.h>
#include <Core/Asset.h>

using namespace Charm;

namespace CharmApp
{
    struct AssetRegistryState
    {
        u32 tableFlags = 0;
        Core::AssetHandle assetToRemove = Core::AssetHandle_Invalid;
    };

    namespace AssetRegistryPanel
    {
        void Init();
        void Display();
    }
}
