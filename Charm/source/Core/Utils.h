#pragma once
#include <string>
#include <filesystem>

namespace Charm
{
    namespace Utils
    {
        inline std::string GetFileName(const char* path, bool hasExtension = false)
        {
            std::filesystem::path p(path);
            std::string fileName = (!hasExtension) ? p.stem().string() : p.filename().string();
            return fileName;
        }
    }
}
