#pragma once
#include "ECS/Scene.h"

#include <entt/entt.hpp>
#include <utility>

using namespace Charm::Core;

namespace Charm
{
    namespace ECS
    {
        namespace Entities
        {
            Entity Create(entt::entity handle, Scene* context);
            Entity FindWithTag(const char* tag, Scene* scene);
            std::vector<Entity> FindEntitiesWithTag(const char* tag, Scene* scene);
        }

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
                return context->registry.get_or_emplace<T>(handle);
            }

            template <typename T>
            inline T* TryGetComponent()
            {
                return context->registry.try_get<T>(handle);
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

        class Scriptable
        {
        public:
            Scriptable() = default;
            virtual ~Scriptable() = default;

            virtual void OnCreate() {};
            virtual void OnDestroy() {};
            virtual void OnUpdate() {};

            inline Entity FindEntityWithTag(const char* tag) { return Entities::FindWithTag(tag, m_entity.context); }
            inline std::vector<Entity> FindEntitiesWithTag(const char* tag) { return Entities::FindEntitiesWithTag(tag, m_entity.context); }

            template <typename T>
            inline T& HasComponent()
            {
                return m_entity.HasComponent<T>();
            }

            template <typename T>
            inline T& GetComponent()
            {
                return m_entity.GetComponent<T>();
            }

            template <typename T>
            inline T* TryGetComponent()
            {
                return m_entity.TryGetComponent<T>();
            }

            template <typename T, typename... Args>
            inline T& AddComponent(Args&&... args)
            {
                return m_entity.AddComponent<T>(std::forward<Args>(args)...);
            }

            template <typename T>
            inline void RemoveComponent()
            {
                m_entity.RemoveComponent<T>();
            }

        private:
            Entity m_entity;

            friend void Scenes::OnRuntimeStart(Scene&);
            friend void Scenes::OnRuntimeStop(Scene&);
        };

    }
}
