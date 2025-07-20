#pragma once
#include "Projects/Project.h"

namespace Charm
{
    namespace Projects
    {
        namespace ProjectSerializer
        {
            void SetContext(Project& project);

            void Serialize(const char* path);
            void SerializeRuntime(const char* path);

            void Deserialize(const char* path);
            void DeserializeRuntime(const char* path);
        }
    }
}
