#pragma once
#include <string>
#include <filesystem>

namespace Charm
{
    namespace Projects
    {
        struct Project
        {
            std::string name = "Untitled";
            std::filesystem::path directory;
            std::filesystem::path assetsDirectory;
            std::filesystem::path startScenePath;
            std::filesystem::path scriptModulePath;
        };

        namespace ProjectManager
        {
            Project New();
            Project Load(const char* path);
            void Save(const char* path, Project& project);

            std::filesystem::path GetAssetPath(const Project& project);
            std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path, const Project& project);
        }
    }
}
