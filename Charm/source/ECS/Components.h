#pragma once
#include "ECS/Entity.h"
#include "ECS/PhysicsWorld.h"
#include "ECS/ScriptManager.h"

#include "Core/AssetManager.h"
#include "Core/Random.h"

#include "Graphics/Animation.h"
#include "Graphics/Camera.h"
#include "Graphics/Shapes.h"
#include "Graphics/Texture.h"

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
            Entity parent = Entity_Null;

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
            s32 sortingLayer = 0;
            Rectangle crop;
            glm::vec2 origin;
            glm::vec4 tint = glm::vec4(1.f);
            OriginMode originMode = OriginMode::Center;

            SpriteRendererComponent() = default;
            SpriteRendererComponent(const SpriteRendererComponent&) = default;
            SpriteRendererComponent(AssetHandle sprite)
            {
                this->sprite = sprite;
                this->crop.width = 1.f;
                this->crop.height = 1.f;

                Texture* texture = AssetManager::GetAsset<Texture>(sprite);

                if (texture != NULL)
                {
                    this->crop.width = texture->width;
                    this->crop.height = texture->height;
                }
            }
        };

        struct CircleRendererComponent
        {
            s32 sortingLayer = 0;
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

        struct Animator2DComponent
        {
            s32 activeSlot = -1;
            AssetHandle controller;

            Animator2DComponent() = default;
            Animator2DComponent(const Animator2DComponent&) = default;
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

        struct Rigidbody2DComponent
        {
            PhysicsBodyType type = PhysicsBodyType::Static;
            bool hasFixedRotation = false;
            float gravityScale = 1.f;
            float linearDamping = 0.f;
            float angularDamping = 0.f;
            glm::vec2 linearVelocity;
            float angularVelocity = 0.f;
            PhysicsBodyID runtimeBody;

            Rigidbody2DComponent() = default;
            Rigidbody2DComponent(const Rigidbody2DComponent&) = default;

            void AddForce(const glm::vec2& force, ForceMode mode);
        };

        struct BoxCollider2DComponent
        {
            bool isTrigger = false;
            glm::vec2 offset;
            glm::vec2 size = glm::vec2(0.5f, 0.5f);
            float density = 1.f;
            float friction = 0.5f;
            float restitution = 0.f;
            PhysicsShapeID runtimeShape;

            BoxCollider2DComponent() = default;
            BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
        };

        struct NativeScriptComponent
        {
            std::string scriptName;
            Scriptable* scriptInstance = NULL;

            ScriptInitFunc CreateScript = NULL;
            ScriptShutdownFunc DestroyScript = NULL;

            NativeScriptComponent() = default;
            NativeScriptComponent(const NativeScriptComponent&) = default;
        };
    }
}
