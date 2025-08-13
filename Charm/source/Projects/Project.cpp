#include "Projects/Project.h"
#include "Projects/ProjectSerializer.h"

namespace Charm
{
    namespace Projects
    {
        namespace ProjectManager
        {
            Project New()
            {
                Project project;
                project.name = "Untitled";
                project.directory = "";
                project.assetsDirectory = "";
                project.startScenePath = "";
                project.scriptModulePath = "";

                return project;
            }

            Project Load(const char* path)
            {
                Project project;
                project.directory = std::filesystem::path(path).parent_path();

                ProjectSerializer::SetContext(project);
                ProjectSerializer::Deserialize(path);

                return project;
            }

            void Save(const char* path, Project& project)
            {
                ProjectSerializer::SetContext(project);
                ProjectSerializer::Serialize(path);

                project.directory = std::filesystem::path(path).parent_path();
            }

            std::filesystem::path GetAssetPath(const Project& project) { return project.directory / project.assetsDirectory; }
            std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path, const Project& project) { return project.directory / project.assetsDirectory / path; }
        }
    }
}
