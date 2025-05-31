#pragma once
#include "Core/Base.h"
#include <string>
#include <vector>

namespace Charm
{
    namespace Core
    {
        struct File
        {
            bool isValid = false;
            u64 size = 0;
            std::string path;
            std::vector<char> data;

            inline std::string asString() const
            {
                return (isValid) ? std::string(data.begin(), data.end()) : "";
            }

            inline const char* asCString() const
            {
                return (isValid) ? data.data() : "";
            }
        };

        namespace IO
        {
            File ReadFile(const char* path);
        }
    }
}
