#include "Projects/Project.h"
#include "Projects/ProjectSerializer.h"
#include "Core/Log.h"

namespace Charm
{
    namespace Projects
    {
        static Project activeProject = ProjectManager::New();

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

            Project Load(const std::filesystem::path& path)
            {
                Project project;
                project.directory = path.parent_path();
                project.path = path;

                ProjectSerializer::SetContext(&project);
                ProjectSerializer::Deserialize(path);
                ProjectSerializer::SetContext(NULL);

                activeProject = project;
                return project;
            }

            void Save(Project& project, const std::filesystem::path& path)
            {
                const std::filesystem::path validPath = (path.empty()) ? project.path : path;

                ProjectSerializer::SetContext(&project);
                ProjectSerializer::Serialize(project.path);
                ProjectSerializer::SetContext(NULL);

                activeProject = project;
            }

            void Log(const Project& project)
            {
                INFO("Project            : %s", project.name.c_str());
                INFO("Path               : %s", project.path.c_str());
                INFO("Directory          : %s", project.directory.c_str());
                INFO("Assets Directory   : %s", project.assetsDirectory.c_str());
                INFO("Start Scene Path   : %s", project.startScenePath.c_str());
                INFO("Script Module Path : %s", project.scriptModulePath.c_str());
            }

            const Project& GetActive() { return activeProject; }
            std::filesystem::path GetScriptModulePath(const Project& project) { return project.directory / project.assetsDirectory / project.scriptModulePath; }
            std::filesystem::path GetStartScenePath(const Project& project) { return project.directory / project.assetsDirectory / project.startScenePath; }
            std::filesystem::path GetAssetPath(const Project& project) { return project.directory / project.assetsDirectory; }
            std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path, const Project& project) { return project.directory / project.assetsDirectory / path; }
            std::filesystem::path GetAssetRelativePath(const std::filesystem::path& path, const Project& project) { return std::filesystem::relative(path, GetAssetPath(project)); }
        }
    }
}
