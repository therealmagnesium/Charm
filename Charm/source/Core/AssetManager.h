#pragma once
#include "Core/Asset.h"
#include "Core/Log.h"

namespace Charm
{
    namespace Core
    {
        struct AssetCollection
        {
            AssetMap loadedAssets;
            AssetRegistry registry;
        };

        namespace AssetManager
        {
            void Init(AssetCollection* collection);
            void Clean();

            AssetHandle Import(const char* path, AssetType type);

            const AssetMap& GetAllAssets();
            const AssetRegistry& GetRegistry();

            Asset* GetAsset(AssetHandle handle);
            bool IsHandleValid(AssetHandle handle);

            template <typename T>
            inline T* GetAsset(AssetHandle handle)
            {
                bool deriveCheck = std::is_base_of_v<Asset, T>;
                ASSERT(deriveCheck, "AssetManager::GetAsset - Must return a type that derives from Asset");

                Asset* asset = GetAsset(handle);
                return dynamic_cast<T*>(asset);
            }
        }
    }
}
