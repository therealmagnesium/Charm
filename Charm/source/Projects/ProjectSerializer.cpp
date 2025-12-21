#include "Projects/ProjectSerializer.h"
#include "Core/Log.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace Charm
{
    namespace Projects
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

                out << YAML::Key << "Project" << YAML::Value << YAML::BeginMap;
                out << YAML::Key << "Name" << YAML::Value << context->name;
                out << YAML::Key << "Start Scene" << YAML::Value << context->startScenePath.string();
                out << YAML::Key << "Assets Directory" << YAML::Value << context->assetsDirectory.string();
                out << YAML::Key << "Script Module" << YAML::Value << context->scriptModulePath.string();
                out << YAML::EndMap;

                /*
                            out << YAML::Key << "Settings" << YAML::Value << YAML::BeginMap;
                            out << YAML::Key << "Editor Grid" << YAML::Value << YAML::BeginMap;
                            out << YAML::EndMap; // Editor Grid
                            out << YAML::EndMap; // Settings*/

                out << YAML::EndMap;

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

                YAML::Node projectNode = data["Project"];
                ASSERT_ERROR(projectNode, "ProjectSerialzier::Deserialize - Invalid project file \"%s\"!", path.c_str());

                context->name = projectNode["Name"].as<std::string>();
                context->startScenePath = projectNode["Start Scene"].as<std::string>();
                context->assetsDirectory = projectNode["Assets Directory"].as<std::string>();
                context->scriptModulePath = projectNode["Script Module"].as<std::string>();
            }

            void DeserializeRuntime(const std::filesystem::path& path) {}
        }
    }
}
