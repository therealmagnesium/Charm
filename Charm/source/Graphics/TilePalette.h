#pragma once
#include "Core/Asset.h"
#include "Core/Base.h"
#include "Graphics/Shapes.h"
#include "Graphics/Texture.h"

#include <vector>
#include <filesystem>

namespace Charm
{
    namespace Graphics
    {
        struct TilePalette : public Core::Asset
        {
            std::vector<Rectangle> crops;
            Core::AssetHandle tileset = Core::AssetHandle_Invalid;
            u32 sliceWidth = 0;
            u32 sliceHeight = 0;
            u32 totalTileCount = 0;

            Core::AssetType GetType() const override { return Core::AssetType::TilePalette; }
        };

        inline const TilePalette TilePalette_Null;

        namespace TilePalettes
        {
            TilePalette Load(const std::filesystem::path& path);
            void Save(const TilePalette& tilePalette, const std::filesystem::path& path = "");
            void Slice(TilePalette& tilePalette, const Texture& tileset, u32 sliceWidth, u32 sliceHeight);
            void Log(const TilePalette& tilePalette);
        }
    }
}
