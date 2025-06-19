#pragma once
#include "Core/Asset.h"
#include <glm/glm.hpp>

using namespace Charm::Core;

namespace Charm
{
    namespace ECS
    {
        struct TransformComponent
        {
            glm::vec3 position;
            glm::vec3 rotation;
            glm::vec3 scale;

            TransformComponent() = default;
            TransformComponent(const TransformComponent&) = default;
            TransformComponent(const glm::vec3& position,
                               const glm::vec3& rotation = glm::vec3(0.f),
                               const glm::vec3& scale = glm::vec3(1.f))
            {
                this->position = position;
                this->rotation = rotation;
                this->scale = scale;
            }
        };

        struct SpriteRendererComponent
        {
            AssetHandle sprite = 0;
            glm::vec3 tint = glm::vec3(1.f);

            SpriteRendererComponent() = default;
            SpriteRendererComponent(const SpriteRendererComponent&) = default;
            SpriteRendererComponent(AssetHandle sprite)
            {
                this->sprite = sprite;
            }
        };
    }
}
