#include "Graphics/TilePalette.h"

#include "Core/AssetManager.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        namespace TilePalettes
        {
            TilePalette Load(const std::filesystem::path& path)
            {
                TilePalette tilePalette;

                YAML::Node data;
                try
                {
                    data = YAML::LoadFile(path);
                }
                catch (const YAML::BadFile& e)
                {
                    ERROR("TilePalettes::Load - Could not load the file \"%s\"!", path.c_str());
                    return tilePalette;
                }

                ASSERT_RETURN(data, tilePalette, "TilePalettes::Load - Failed to load tile palette \"%s\", the path may be invalid!", path.c_str());
                ASSERT_RETURN(data["Tile Palette"], tilePalette, "TilePalettes::Load - Failed to load tile palette \"%s\", the path may be an invalid animation file!", path.c_str());

                const YAML::Node& properties = data["Properties"];
                tilePalette.sliceWidth = properties["Slice Width"].as<u32>();
                tilePalette.sliceHeight = properties["Slice Height"].as<u32>();
                tilePalette.totalTileCount = properties["Tile Count"].as<u32>();
                tilePalette.isValid = true;

                INFO("Tile palette \"%s\" was loaded successfully", path.c_str());
                return tilePalette;
            }

            void Save(const TilePalette& tilePalette, const std::filesystem::path& path)
            {
                YAML::Emitter out;
                out << YAML::BeginMap; // Root

                out << YAML::Key << "Tile Palette" << YAML::Value << tilePalette.handle;
                out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap; // Properties
                out << YAML::Key << "Slice Width" << YAML::Value << tilePalette.sliceWidth;
                out << YAML::Key << "Slice Height" << YAML::Value << tilePalette.sliceHeight;
                out << YAML::Key << "Tile Count" << YAML::Value << tilePalette.totalTileCount;
                out << YAML::EndMap; // Properties
                out << YAML::EndMap; // Root

                const std::filesystem::path tilePalettePath = AssetManager::GetAssetPathAbsolute(tilePalette.handle);
                const std::filesystem::path validPath = (!path.empty()) ? path : tilePalettePath;
                std::ofstream fout(validPath.c_str());
                if (fout.is_open())
                {
                    fout << out.c_str() << "\n";
                    fout.close();

                    INFO("Tile palette \"%s\" was saved successfully", validPath.c_str());
                }
                else
                    ERROR("TilePalettes::Save - Failed to save tile palette \"%s\", the path may be invalid!", validPath.c_str());
            }

            void Slice(TilePalette& tilePalette, const Texture& tileset, u32 sliceWidth, u32 sliceHeight)
            {
                ASSERT_ERROR(AssetManager::IsHandleValid(tilePalette.handle), "TilePalettes::Slice - Invalid tile palette handle!");
                ASSERT_ERROR(AssetManager::IsHandleValid(tileset.handle), "TilePalettes::Slice - Invalid tileset handle!");

                const u32 numColumns = tileset.width / sliceWidth;
                const u32 numRows = tileset.height / sliceHeight;

                tilePalette.totalTileCount = numRows * numColumns;
                tilePalette.sliceWidth = sliceWidth;
                tilePalette.sliceHeight = sliceHeight;
                tilePalette.tileset = tileset.handle;
            }

            void Log(const TilePalette& tilePalette)
            {
                INFO("Tile Palette     : 0x%lx", tilePalette.handle);
                INFO("Slice Width      : %d", tilePalette.sliceWidth);
                INFO("Slice Height     : %d", tilePalette.sliceHeight);
                INFO("Total Tile Count : %d", tilePalette.totalTileCount);
                INFO("Is Valid?        : %s", Utils::BoolToCString(tilePalette.isValid));
            }
        }
    }
}
