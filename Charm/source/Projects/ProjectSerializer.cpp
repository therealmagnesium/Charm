#include "Projects/ProjectSerializer.h"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

using namespace Charm::Core;

namespace Charm::Projects
{
    static Project* context = NULL;

    namespace ProjectSerializer
    {
        void SetContext(Project* project) { context = project; }
        const Project& GetContext() { return (context != NULL) ? *context : Project_Null; }

        void Serialize(const std::filesystem::path& path)
        {
            ASSERT_ERROR(context != NULL, "ProjectSerializer::Serialize - The context has not been set!");

            YAML::Emitter out;
            out << YAML::BeginMap;

            out << YAML::Key << "Project" << YAML::Value << YAML::BeginMap; // Project
            out << YAML::Key << "Name" << YAML::Value << context->name;
            out << YAML::Key << "Type" << YAML::Value << Utils::ProjectTypeToString(context->type);
            out << YAML::Key << "Start Scene" << YAML::Value << context->startScenePath.string();
            out << YAML::Key << "Assets Directory" << YAML::Value << context->assetsDirectory.string();
            out << YAML::Key << "Script Module" << YAML::Value << context->scriptModulePath.string();
            out << YAML::EndMap; // Project

            out << YAML::Key << "Settings" << YAML::Value << YAML::BeginMap; // Settings
            out << YAML::Key << "General" << YAML::Value << YAML::BeginMap;  // General
            out << YAML::Key << "Pixels Per Unit" << YAML::Value << Application::GetPixelsPerUnit();
            out << YAML::EndMap;                                                // General
            out << YAML::Key << "Editor Grid" << YAML::Value << YAML::BeginMap; // Editor Grid
            out << YAML::Key << "Is Enabled?" << YAML::Value << context->grid.isEnabled;
            out << YAML::Key << "Tile Scale" << YAML::Value << context->grid.tileScale;
            out << YAML::EndMap; // Editor Grid
            out << YAML::EndMap; // Settings

            out << YAML::EndMap; // Root

            std::ofstream fout(path);
            if (fout.is_open())
            {
                fout << out.c_str() << "\n";
                fout.close();
            }
            else
                ERROR("ProjectSerializer::Serialize - Failed to save project %s, the path may be invalid!", path.c_str());
        }

        void SerializeRuntime(const std::filesystem::path& path) {}

        void Deserialize(const std::filesystem::path& path)
        {
            ASSERT_ERROR(context != NULL, "ProjectSerializer::Deserialize - The context has not been set!");

            std::stringstream stream;
            std::ifstream in(path);

            ASSERT_ERROR(in.is_open(), "ProjectSerializer::Deserialize - Failed to load project \"%s\"!", path.c_str());

            stream << in.rdbuf();
            in.close();

            YAML::Node data = YAML::Load(stream.str());
            ASSERT_ERROR(data, "ProjectSerialzier::Deserialize - Failed to load project \"%s\"!", path.c_str());

            const YAML::Node& projectNode = data["Project"];
            const YAML::Node& settingsNode = data["Settings"];

            ASSERT_ERROR(projectNode && settingsNode, "ProjectSerialzier::Deserialize - Invalid project file \"%s\"!", path.c_str());

            context->name = projectNode["Name"].as<std::string>();
            context->type = Utils::StringToProjectType(projectNode["Type"].as<std::string>());
            context->startScenePath = projectNode["Start Scene"].as<std::string>();
            context->assetsDirectory = projectNode["Assets Directory"].as<std::string>();
            context->scriptModulePath = projectNode["Script Module"].as<std::string>();

            const YAML::Node& generalSettingsNode = settingsNode["General"];
            const YAML::Node& gridSettingsNode = settingsNode["Editor Grid"];
            context->grid.isEnabled = gridSettingsNode["Is Enabled?"].as<bool>();
            context->grid.tileScale = gridSettingsNode["Tile Scale"].as<u32>();

            const u32 pixelsPerUnit = generalSettingsNode["Pixels Per Unit"].as<u32>();
            Application::SetPixelsPerUnit(pixelsPerUnit);
        }

        void DeserializeRuntime(const std::filesystem::path& path) {}
    }
}
