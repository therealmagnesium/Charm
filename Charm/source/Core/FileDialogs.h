#pragma once
#include "Core/Base.h"

#include <filesystem>
#include <vector>

namespace Charm
{
    namespace Core
    {
        struct FileDialogFilter
        {
            const char* name = "";
            const char* specification = "";

            FileDialogFilter() = default;
            FileDialogFilter(const char* name, const char* specification)
            {
                this->name = name;
                this->specification = specification;
            }
        };

        struct FileDialogState
        {
            std::filesystem::path defaultPath;
            std::vector<std::filesystem::path> selectedPaths;
        };

        namespace FileDialogs
        {
            void Init();
            void Shutdown();

            bool Open(FileDialogFilter* filters = NULL, u8 filterCount = 0);
            bool OpenMultiple();
            bool Save(FileDialogFilter* filters, u8 filterCount);

            std::filesystem::path GetSelectedPath();
            const std::vector<std::filesystem::path>& GetSelectedPathMulti();

            void SetDefaultPath(const std::filesystem::path& path);
        }
    }
}
