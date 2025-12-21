#pragma once
#include "Projects/Project.h"

namespace Charm
{
    namespace Projects
    {
        namespace ProjectSerializer
        {
            void Serialize(const std::filesystem::path& path);
            void SerializeRuntime(const std::filesystem::path& path);

            void Deserialize(const std::filesystem::path& path);
            void DeserializeRuntime(const std::filesystem::path& path);

            void SetContext(Project* project);
            const Project& GetContext();
        }
    }
}
