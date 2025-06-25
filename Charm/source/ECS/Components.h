#pragma once
#include "Core/Asset.h"
#include "Core/Random.h"

#include "Graphics/Camera.h"

#include <glm/glm.hpp>
#include <string>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace ECS
    {
        struct InternalComponent
        {
            UUID id = 0;
            bool isActive = false;
            std::string tag = "Entity";

            InternalComponent()
            {
                id = Random::GenerateUUID();
                isActive = true;
            }

            InternalComponent(const InternalComponent&) = default;

            InternalComponent(UUID id, const char* tag)
            {
                this->id = id;
                this->tag = tag;
                this->isActive = true;
            }
        };

        struct TransformComponent
        {
            glm::vec3 position = glm::vec3(0.f);
            glm::vec3 rotation = glm::vec3(0.f);
            glm::vec3 scale = glm::vec3(1.f);

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

        struct CircleRendererComponent
        {
            float radius = 1.f;
            float thickness = 1.f;
            float fade = 0.05f;
            glm::vec3 color = glm::vec3(1.f);

            CircleRendererComponent() = default;
            CircleRendererComponent(const CircleRendererComponent&) = default;
            CircleRendererComponent(float radius, float thickness, float fade, const glm::vec3& color = glm::vec3(1.f))
            {
                this->radius = radius;
                this->thickness = thickness;
                this->fade = fade;
                this->color = color;
            }
        };

        struct Camera2DComponent
        {
            bool isPrimary = false;
            Camera2D camera;

            Camera2DComponent() = default;
            Camera2DComponent(const Camera2DComponent&) = default;
        };

        /*  TODO: Implement later
            struct Camera3DComponent
            {
            };
        */
    }
}
