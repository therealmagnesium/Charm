#pragma once
#include <filesystem>
#include <vector>

namespace Charm
{
    namespace Core
    {
        struct FileDialogState
        {
            std::filesystem::path defaultPath;
            std::vector<std::filesystem::path> selectedPaths;
        };

        namespace FileDialogs
        {
            void Init();
            void Shutdown();

            bool Open();
            bool OpenMultiple();
            bool Save();

            std::filesystem::path GetSelectedPath();
            const std::vector<std::filesystem::path>& GetSelectedPathMulti();

            void SetDefaultPath(const std::filesystem::path& path);
        }
    }
}
