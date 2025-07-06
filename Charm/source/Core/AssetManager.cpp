#include "Core/AssetManager.h"
#include "Core/Asset.h"
#include "Core/Log.h"
#include "Core/Random.h"

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

                AssetMetadata metadata;
                metadata.path = path;
                metadata.type = type;

                Asset* asset = LoadAsset(handle, metadata);
                if (asset != NULL)
                {
                    assets->registry[handle] = metadata;
                    assets->loadedAssets[handle] = asset;
                }

                return handle;
            }

            void Import(const char* path, AssetType type, AssetHandle handle)
            {
                AssetMetadata metadata;
                metadata.path = path;
                metadata.type = type;

                Asset* asset = LoadAsset(handle, metadata);
                if (asset != NULL)
                {
                    assets->registry[handle] = metadata;
                    assets->loadedAssets[handle] = asset;
                }
            }

            const AssetMap& GetAllAssets() { return assets->loadedAssets; }
            const AssetRegistry& GetRegistry() { return assets->registry; }

            Asset* GetAsset(AssetHandle handle)
            {
                if (!IsHandleValid(handle))
                    return NULL;

                Asset* asset = assets->loadedAssets[handle];
                return asset;
            }

            bool IsHandleValid(AssetHandle handle)
            {
                return handle != 0 && assets->registry.find(handle) != assets->registry.end();
            }

            Asset* LoadAsset(AssetHandle handle, AssetMetadata& metadata)
            {
                Asset* asset = NULL;

                switch (metadata.type)
                {
                    case AssetType::Texture:
                    {
                        Texture texture = Textures::Load(metadata.path.c_str());
                        asset = new Texture(std::move(texture));
                        asset->handle = handle;
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
