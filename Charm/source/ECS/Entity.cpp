#include "ECS/Entity.h"
#include "ECS/Components.h"
#include "ECS/Scene.h"

#include "Core/Log.h"

#include <entt/entt.hpp>

using namespace Charm::Core;

namespace Charm
{
    namespace ECS
    {
        namespace Entities
        {
            Entity Create(entt::entity handle, Scene* context)
            {
                Entity entity;
                entity.handle = handle;
                entity.context = context;

                return entity;
            }

            Entity FindWithTag(const char* tag, Scene* scene)
            {
                std::vector<Entity> entities = FindEntitiesWithTag(tag, scene);
                return (entities.size() >= 1) ? entities[0] : (Entity){};
            }

            Entity FindWithUUID(UUID uuid, Scene* scene)
            {
                auto entities = scene->registry.view<InternalComponent>();
                Entity match;

                for (auto entityID : entities)
                {
                    Entity entity = Entities::Create(entityID, scene);
                    auto& internal = entity.GetComponent<InternalComponent>();

                    if (internal.id == uuid)
                    {
                        match = entity;
                        break;
                    }
                }

                if (!match)
                    ERROR("Entities::FindWithUUID - Could not find entity with UUID %ld", uuid);

                return match;
            }

            std::vector<Entity> FindEntitiesWithTag(const char* tag, Scene* scene)
            {
                std::vector<Entity> tagged;
                tagged.reserve(scene->entityCount);

                if (scene == NULL)
                {
                    ERROR("Entities::FindEntitiesWithTag - The scene reference to search is null!");
                    return tagged;
                }

                auto entities = scene->registry.view<InternalComponent>();
                for (auto entityID : entities)
                {
                    Entity entity = Entities::Create(entityID, scene);
                    auto& internal = entity.GetComponent<InternalComponent>();
                    std::string compareTag = tag;
                    if (internal.tag == compareTag)
                        tagged.emplace_back(entity);
                }

                return tagged;
            }
        }
    }
}
