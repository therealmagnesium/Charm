#pragma once
#include "Core/Asset.h"
#include "Graphics/Mesh.h"

namespace Charm
{
    namespace Graphics
    {
        struct Model : public Core::Asset
        {
            std::vector<Mesh> meshes;
            std::vector<Material> materials;
            bool isDynamic = false;

            virtual Core::AssetType GetType() const override { return Core::AssetType::Model; }
        };

        namespace Models
        {
            Model Load(const std::filesystem::path& path);
            void Unload(Model& model);
        }
    }
}
