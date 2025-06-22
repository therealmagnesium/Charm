#pragma once
#include "ECS/Scene.h"

namespace Charm
{
    namespace ECS
    {
        namespace SceneSerializer
        {
            void SetContext(Scene& scene);

            void Serialize(const char* path);
            void SerializeRuntime(const char* path);

            void Deserialize(const char* path);
            void DeserializeRuntime(const char* path);
        }
    }
}
