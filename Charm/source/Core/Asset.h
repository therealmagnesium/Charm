#pragma once
#include "Core/Base.h"
#include <string>
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
            Texture,
            Shader,
        };

        struct Asset
        {
            AssetHandle handle = 0;

            virtual AssetType GetType() { return AssetType::Invalid; }
        };

        struct AssetMetadata
        {
            std::string path = "";
            AssetType type = AssetType::Invalid;
        };
    }
}
