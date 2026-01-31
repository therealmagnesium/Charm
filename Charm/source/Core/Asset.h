#pragma once
#include "Core/Base.h"
#include <filesystem>
#include <unordered_map>

namespace Charm
{
    namespace Core
    {
        using AssetHandle = u64;
        using AssetRegistry = std::unordered_map<AssetHandle, struct AssetMetadata>;
        using AssetMap = std::unordered_map<AssetHandle, struct Asset*>;

        enum class AssetType : u8
        {
            Invalid = 0,
            Animation,
            AnimationController,
            Material,
            Model,
            Shader,
            Texture,
            TilePalette,
            _TotalCount,
        };

        struct Asset
        {
            AssetHandle handle = 0;
            bool isValid = false;

            virtual AssetType GetType() const { return AssetType::Invalid; }
        };

        struct AssetMetadata
        {
            std::filesystem::path path;
            AssetType type = AssetType::Invalid;
        };

        inline const AssetHandle AssetHandle_Invalid = 0;
    }
}
