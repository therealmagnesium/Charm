#include "Graphics/Material.h"
#include "Graphics/Renderer.h"
#include "Core/AssetManager.h"
#include "Core/IO.h"
#include "Core/Log.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

using namespace Charm::Core;

namespace Charm::Graphics
{
    namespace Materials
    {
        Material Load(const std::filesystem::path& path)
        {
            Material material;
            ASSERT_RETURN(std::filesystem::exists(path), material,
                          "Materials::Load - Could not load \"%s\" because the path does not exist",
                          path.c_str());

            YAML::Node data;
            try
            {
                data = YAML::LoadFile(path);
            }
            catch (...)
            {
                ERROR("Materials::Load - Failed loading the file \"%s\"!", path.c_str());
                return material;
            }

            ASSERT_RETURN(data, material, "Materials::Load - Failed to load material \"%s\", the path may be invalid!", path.c_str());
            ASSERT_RETURN(data["Material"], material, "Materials::Load - Failed to load the material \"%s\", the path may be an invalid material file!", path.c_str());

            const YAML::Node& materialNode = data["Material"];
            const AssetHandle albedoTextureHandle = materialNode["Albedo Texture"].as<AssetHandle>();
            material.albedo = materialNode["Albedo"].as<glm::vec4>();
            material.albedoTexture = AssetManager::GetAsset<Texture>(albedoTextureHandle);
            material.name = materialNode["Name"].as<std::string>();
            material.shader = &Renderer::GetShaderBlinnPhong();
            material.isValid = true;

            INFO("Material \"%s\" was loaded successfully", path.c_str());
            return material;
        }

        void Save(const Material& material, const std::filesystem::path& path)
        {
            const bool hasValidAlbedoTexture = material.albedoTexture != NULL;
            const AssetHandle validAlbedoTextureHandle = hasValidAlbedoTexture ? material.albedoTexture->handle : AssetHandle_Invalid;

            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Material" << YAML::Value << YAML::BeginMap;
            out << YAML::Key << "Name" << YAML::Value << material.name;
            out << YAML::Key << "Handle" << YAML::Value << material.handle;
            out << YAML::Key << "Albedo" << YAML::Value << material.albedo;
            out << YAML::Key << "Albedo Texture" << YAML::Value << validAlbedoTextureHandle;
            out << YAML::EndMap;

            out << YAML::EndMap;

            std::ofstream file(path);
            file << out.c_str();
            file.close();
        }
    }
}
