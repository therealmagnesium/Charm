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
            void Import(const char* path, AssetType type, AssetHandle handle);

            const AssetMap& GetAllAssets();
            const AssetRegistry& GetRegistry();

            Asset* GetAsset(AssetHandle handle);
            std::filesystem::path GetAssetPath(AssetHandle handle);
            bool IsHandleValid(AssetHandle handle);
            bool IsAssetLoaded(AssetHandle handle);
            bool IsAssetRegistered(const std::string& path);
            AssetHandle FindAssetHandle(const std::string& path);

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
