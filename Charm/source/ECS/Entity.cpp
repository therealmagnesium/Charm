#include "ECS/Entity.h"
#include "ECS/Scene.h"

#include "Core/Random.h"

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
        }
    }
}
