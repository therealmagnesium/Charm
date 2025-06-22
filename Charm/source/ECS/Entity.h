#pragma once
#include "ECS/Scene.h"
#include "Core/Random.h"

#include <entt/entt.hpp>
#include <string>
#include <utility>

using namespace Charm::Core;

namespace Charm
{
    namespace ECS
    {
        struct Entity
        {
            Scene* context = NULL;
            entt::entity handle = entt::null;

            template <typename T>
            inline bool HasComponent()
            {
                return context->registry.all_of<T>(handle);
            }

            template <typename T>
            inline T& GetComponent()
            {
                return context->registry.get<T>(handle);
            }

            template <typename T, typename... Args>
            inline T& AddComponent(Args&&... args)
            {
                return context->registry.emplace_or_replace<T>(handle, std::forward<Args>(args)...);
            }

            template <typename T>
            inline void RemoveComponent()
            {
                context->registry.remove<T>(handle);
            }

            inline operator bool() const { return handle != entt::null && context != NULL; }
            inline bool operator==(const Entity& other) { return handle == other.handle && context == other.context; }
            inline bool operator!=(const Entity& other) { return handle != other.handle || context != other.context; }
        };

        namespace Entities
        {
            Entity Create(entt::entity handle, Scene* context);
        }
    }
}
