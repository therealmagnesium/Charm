#pragma once
#include <string>
#include <vector>

namespace Charm
{
    namespace Core
    {
        struct FileDialogState
        {
            std::string defaultPath;
            std::vector<std::string> selectedPaths;
        };

        namespace FileDialogs
        {
            void Init();
            void Shutdown();

            bool Open();
            bool OpenMultiple();
            bool Save();

            const std::string& GetSelectedPath();
            const std::vector<std::string>& GetSelectedPathMulti();
        }
    }
}
