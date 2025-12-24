#pragma once
#include "Core/Base.h"

#include <string>
#include <filesystem>

namespace Charm
{
    namespace Projects
    {
        struct GridSettings
        {
            bool isEnabled = true;
            u32 tileScale = 1;
        };

        struct Project
        {
            std::string name = "Untitled";
            std::filesystem::path path;
            std::filesystem::path directory;
            std::filesystem::path assetsDirectory;
            std::filesystem::path startScenePath;
            std::filesystem::path scriptModulePath;
            GridSettings grid;

            bool operator==(const Project& other) const
            {
                bool isTheSame = name == other.name && path == other.path &&
                                 directory == other.directory && assetsDirectory == other.assetsDirectory &&
                                 startScenePath == other.startScenePath && scriptModulePath == other.scriptModulePath;
                return isTheSame;
            }

            bool operator!=(const Project& other) const { return !((*this) == other); }
        };

        inline const Project Project_Null;

        namespace ProjectManager
        {
            Project New();
            Project Load(const std::filesystem::path& path);
            void Save(Project& project, const std::filesystem::path& path = "");
            void Log(const Project& project);

            const Project& GetActive();
            std::filesystem::path GetScriptModulePath(const Project& project);
            std::filesystem::path GetStartScenePath(const Project& project);
            std::filesystem::path GetAssetPath(const Project& project);
            std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path, const Project& project);
            std::filesystem::path GetAssetRelativePath(const std::filesystem::path& path, const Project& project);
        }
    }
}
