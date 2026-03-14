#pragma once
#include "Core/Asset.h"
#include "Graphics/Texture.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Charm::Graphics
{
    struct Shader;

    struct Material : public Core::Asset
    {
        std::string name = "Material";
        glm::vec4 albedo = glm::vec4(1.f);
        Texture* albedoTexture = NULL;
        Shader* shader = NULL;

        inline virtual Core::AssetType GetType() const override { return Core::AssetType::Material; }
        inline bool IsTranslucent() const
        {
            const bool colorHasTransparency = albedo.a < 1.f;
            const bool textureHasTransparency = albedoTexture != NULL && albedoTexture->hasTransparency;
            return colorHasTransparency || textureHasTransparency;
        }
    };

    namespace Materials
    {
        Material Load(const std::filesystem::path& path);
        void Save(const Material& material, const std::filesystem::path& path);
    }
}
