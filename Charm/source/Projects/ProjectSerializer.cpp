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
            void SetContext(Project& project) { context = &project; }

            void Serialize(const char* path)
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

                out << YAML::EndMap;

                std::ofstream fout(path);
                if (fout.is_open())
                {
                    fout << out.c_str() << "\n";
                    fout.close();
                }
                else
                    ERROR("ProjectSerializer::Serialize - Failed to save project %s, the path may be invalid!", path);
            }

            void SerializeRuntime(const char* path) {}

            void Deserialize(const char* path)
            {
                ASSERT_ERROR(context != NULL, "ProjectSerializer::Deserialize - The context has not been set!");

                YAML::Node data = YAML::LoadFile(path);
                ASSERT_ERROR(data, "ProjectSerialzier::Deserialize - Failed to load project %s!", path);

                YAML::Node projectNode = data["Project"];
                ASSERT_ERROR(projectNode, "ProjectSerialzier::Deserialize - Invalid project file %s!", path);

                context->name = projectNode["Name"].as<std::string>();
                context->startScenePath = projectNode["Start Scene"].as<std::string>();
                context->assetsDirectory = projectNode["Assets Directory"].as<std::string>();
                context->scriptModulePath = projectNode["Script Module"].as<std::string>();
            }

            void DeserializeRuntime(const char* path) {}
        }
    }
}
