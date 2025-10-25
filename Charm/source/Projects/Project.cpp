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
                project.name = "New Project";
                project.assetsDirectory = "Assets";
                project.directory = "";
                project.startScenePath = "Scenes/SampleScene.charm";

#ifdef CH_PLATFORM_LINUX
                project.scriptModulePath = "Scripts/Binaries/libCharmScriptModule.so";
#endif
#ifdef CH_PLATFORM_WINDOWS
                project.scriptModulePath = "Scripts/Binaries/libCharmScriptModule.dll";
#endif

                return project;
            }

            Project Load(const char* path)
            {
                Project project;
                project.directory = std::filesystem::absolute(std::filesystem::path(path).parent_path());

                ProjectSerializer::SetContext(&project);
                ProjectSerializer::Deserialize(path);
                ProjectSerializer::SetContext(NULL);

                return project;
            }

            void Save(const char* path, Project& project)
            {
                project.directory = std::filesystem::absolute(std::filesystem::path(path).parent_path());

                ProjectSerializer::SetContext(&project);
                ProjectSerializer::Serialize(path);
                ProjectSerializer::SetContext(NULL);
            }

            std::filesystem::path GetScriptModulePath(const Project& project) { return project.directory / project.assetsDirectory / project.scriptModulePath; }
            std::filesystem::path GetStartScenePath(const Project& project) { return project.directory / project.assetsDirectory / project.startScenePath; }
            std::filesystem::path GetAssetPath(const Project& project) { return project.directory / project.assetsDirectory; }
            std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path, const Project& project) { return project.directory / project.assetsDirectory / path; }
        }
    }
}
