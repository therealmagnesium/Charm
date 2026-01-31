#include "Core/AssetManager.h"
#include "Core/Asset.h"
#include "Core/Log.h"
#include "Core/Random.h"
#include "Core/Utils.h"

#include "Graphics/Animation.h"
#include "Graphics/Model.h"
#include "Graphics/Texture.h"
#include "Graphics/TilePalette.h"

#include "Projects/Project.h"
#include <algorithm>

using namespace Charm::Graphics;
using namespace Charm::Projects;

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

                        case AssetType::Model:
                            Models::Unload(dynamic_cast<Model&>(*asset));
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

            AssetHandle Import(const std::filesystem::path& path, AssetType type)
            {
                const Project& project = ProjectManager::GetActive();
                const std::filesystem::path relativePath = ProjectManager::GetAssetRelativePath(path, project);

                if (IsAssetRegistered(relativePath))
                {
                    const AssetHandle handle = FindAssetHandle(relativePath);
                    return handle;
                }

                const AssetHandle handle = Random::GenerateUUID();
                AssetManager::Import(relativePath, type, handle);
                return handle;
            }

            void Import(const std::filesystem::path& path, AssetType type, AssetHandle handle)
            {
                AssetMetadata metadata;
                metadata.path = path;
                metadata.type = type;

                if (metadata.type == AssetType::Invalid)
                    return;

                Asset* asset = (IsAssetRegistered(metadata.path) && IsAssetLoaded(handle)) ? assets->loadedAssets.at(handle) : LoadAsset(handle, metadata);
                if (asset != NULL)
                {
                    const Project& project = ProjectManager::GetActive();
                    metadata.path = ProjectManager::GetAssetRelativePath(path, project);
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
            std::filesystem::path GetAssetPathAbsolute(AssetHandle handle)
            {
                const Project& project = ProjectManager::GetActive();
                const std::filesystem::path relativePath = GetAssetPath(handle);
                const std::filesystem::path absolutePath = ProjectManager::GetAssetFileSystemPath(relativePath, project);
                return absolutePath;
            }

            bool IsAssetTypeRegistered(AssetType type)
            {
                auto HasType = [=](const std::pair<AssetHandle, AssetMetadata>& pair) { return pair.second.type == type; };
                auto it = std::find_if(assets->registry.begin(), assets->registry.end(), HasType);
                return it != assets->registry.end();
            }

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
                auto IsPathRegistered = [&](const std::pair<AssetHandle, AssetMetadata>& pair) { return pair.second.path == path; };
                auto it = std::find_if(assets->registry.begin(), assets->registry.end(), IsPathRegistered);
                return it != assets->registry.end();
            }

            AssetHandle FindAssetHandle(const std::filesystem::path& path)
            {
                AssetHandle searchedAssetHandle = AssetHandle_Invalid;
                auto IsPathRegistered = [&](const std::pair<AssetHandle, AssetMetadata>& pair) { return pair.second.path == path; };
                auto it = std::find_if(assets->registry.begin(), assets->registry.end(), IsPathRegistered);

                if (it != assets->registry.end())
                    searchedAssetHandle = it->first;

                return searchedAssetHandle;
            }

            std::vector<AssetHandle> GetAllHandlesOfType(AssetType type)
            {
                std::vector<AssetHandle> handles;
                handles.reserve(assets->registry.size());

                for (const auto& [handle, metadata] : assets->registry)
                    if (metadata.type == type)
                        handles.emplace_back(handle);

                return handles;
            }

            Asset* LoadAsset(AssetHandle handle, AssetMetadata& metadata)
            {
                const Project& project = ProjectManager::GetActive();
                const std::filesystem::path path = ProjectManager::GetAssetFileSystemPath(metadata.path, project);
                Asset* asset = NULL;

                switch (metadata.type)
                {
                    case AssetType::Texture:
                    {
                        Texture texture = Textures::Load(path.c_str());
                        if (texture.isValid)
                        {
                            asset = new Texture(std::move(texture));
                            asset->handle = handle;
                        }
                        break;
                    }

                    case AssetType::Animation:
                    {
                        Animation animation = Animations::Load(path.c_str());
                        if (animation.isValid)
                        {
                            asset = new Animation(std::move(animation));
                            asset->handle = handle;
                        }
                        break;
                    }

                    case AssetType::AnimationController:
                    {
                        AnimationController controller = Animations::LoadController(path.c_str());
                        if (controller.isValid)
                        {
                            asset = new AnimationController(std::move(controller));
                            asset->handle = handle;
                        }
                        break;
                    }

                    case AssetType::TilePalette:
                    {
                        TilePalette tilePalette = TilePalettes::Load(path);
                        if (tilePalette.isValid)
                        {
                            asset = new TilePalette(std::move(tilePalette));
                            asset->handle = handle;
                        }
                        break;
                    }

                    case AssetType::Model:
                    {
                        Model model = Models::Load(path);
                        if (model.isValid)
                        {
                            asset = new Model(std::move(model));
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
