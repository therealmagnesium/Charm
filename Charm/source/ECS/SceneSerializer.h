#pragma once
#include "ECS/Scene.h"
#include <filesystem>

namespace Charm
{
    namespace ECS
    {
        namespace SceneSerializer
        {
            void SetContext(Scene* scene);

            void Serialize(const std::filesystem::path& path);
            void SerializeRuntime(const std::filesystem::path& path);

            void Deserialize(const std::filesystem::path& path);
            void DeserializeRuntime(const std::filesystem::path& path);
        }
    }
}
