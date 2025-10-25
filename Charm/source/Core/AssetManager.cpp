#include "Core/AssetManager.h"
#include "Core/Asset.h"
#include "Core/Log.h"
#include "Core/Random.h"

#include "Graphics/Animation.h"
#include "Graphics/Texture.h"

using namespace Charm::Graphics;

namespace Charm
{
    namespace Core
    {
        static AssetCollection* assets = NULL;

        namespace AssetManager
        {
            Asset* LoadAsset(AssetHandle handle, AssetMetadata& metadata);

            void Init(AssetCollection* collection)
            {
                if (assets != NULL)
                {
                    WARN("Cannot initialize the asset manager more than once");
                    return;
                }

                assets = collection;
                INFO("The asset manager was successfully initialized");
            }

            void Clean()
            {
                INFO("The asset manager is unloading assets...");

                for (auto& [handle, asset] : assets->loadedAssets)
                {
                    switch (asset->GetType())
                    {
                        case AssetType::Texture:
                            Textures::Unload(dynamic_cast<Texture&>(*asset));
                            break;

                        default:
                            break;
                    }

                    delete asset;
                    asset = NULL;
                }

                assets->loadedAssets.clear();
                assets->registry.clear();
            }

            AssetHandle Import(const char* path, AssetType type)
            {
                AssetHandle handle = Random::GenerateUUID();
                AssetManager::Import(path, type, handle);
                return handle;
            }

            void Import(const char* path, AssetType type, AssetHandle handle)
            {
                AssetMetadata metadata;
                metadata.path = path;
                metadata.type = type;

                if (metadata.type == AssetType::Invalid)
                    return;

                Asset* asset = (IsAssetRegistered(metadata.path) && IsAssetLoaded(handle)) ? assets->loadedAssets[handle] : LoadAsset(handle, metadata);
                if (asset != NULL)
                {
                    assets->registry[handle] = metadata;
                    assets->loadedAssets[handle] = asset;
                }
            }

            void Remove(AssetHandle handle)
            {
                const std::filesystem::path assetPath = assets->registry[handle].path;

                if (AssetManager::IsHandleValid(handle))
                {
                    INFO("Removing asset \"%s\"...", assetPath.c_str());
                    assets->registry.erase(handle);
                    assets->loadedAssets.erase(handle);
                }
            }

            const AssetMap& GetAllAssets() { return assets->loadedAssets; }
            const AssetRegistry& GetRegistry() { return assets->registry; }

            u32 GetTotalAssetCount() { return assets->registry.size() == assets->loadedAssets.size() ? assets->registry.size() : assets->loadedAssets.size(); }

            Asset* GetAsset(AssetHandle handle)
            {
                if (!IsHandleValid(handle))
                    return NULL;

                Asset* asset = assets->loadedAssets[handle];
                return asset;
            }

            AssetType GetAssetType(AssetHandle handle)
            {
                AssetType type = AssetType::Invalid;
                Asset* asset = GetAsset(handle);

                if (asset != NULL)
                    type = asset->GetType();

                return type;
            }

            std::filesystem::path GetAssetPath(AssetHandle handle) { return assets->registry.at(handle).path; }

            bool IsHandleValid(AssetHandle handle)
            {
                return handle != 0 && assets->registry.find(handle) != assets->registry.end();
            }

            bool IsAssetLoaded(AssetHandle handle)
            {
                return assets->loadedAssets.find(handle) != assets->loadedAssets.end();
            }

            bool IsAssetRegistered(const std::filesystem::path& path)
            {
                bool isRegistered = false;
                for (auto& [handle, metadata] : assets->registry)
                {
                    if (metadata.path == path)
                    {
                        isRegistered = true;
                        break;
                    }
                }

                return isRegistered;
            }

            AssetHandle FindAssetHandle(const std::filesystem::path& path)
            {
                AssetHandle searchedAssetHandle = AssetHandle_Invalid;
                for (auto& [handle, metadata] : assets->registry)
                {
                    if (metadata.path == path)
                    {
                        searchedAssetHandle = handle;
                        break;
                    }
                }

                return searchedAssetHandle;
            }

            Asset* LoadAsset(AssetHandle handle, AssetMetadata& metadata)
            {
                Asset* asset = NULL;

                switch (metadata.type)
                {
                    case AssetType::Texture:
                    {
                        Texture texture = Textures::Load(metadata.path.c_str());
                        if (texture.isValid)
                        {
                            asset = new Texture(std::move(texture));
                            asset->handle = handle;
                        }
                        break;
                    }

                    case AssetType::Animation:
                    {
                        Animation animation = Animations::Load(metadata.path.c_str());
                        if (animation.isValid)
                        {
                            asset = new Animation(std::move(animation));
                            asset->handle = handle;
                        }
                        break;
                    }

                    case AssetType::AnimationController:
                    {
                        AnimationController controller = Animations::LoadController(metadata.path.c_str());
                        if (controller.isValid)
                        {
                            asset = new AnimationController(std::move(controller));
                            asset->handle = handle;
                        }
                        break;
                    }

                    default:
                        break;
                }

                return asset;
            }
        }
    }
}
