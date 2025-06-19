#pragma once
#include "Graphics/Camera.h"
#include <entt/entt.hpp>

using namespace Charm::Graphics;

namespace Charm
{
    namespace ECS
    {
        struct Entity;

        struct Scene
        {
            entt::registry registry;
            Camera2D editorCamera2D;
        };

        namespace Scenes
        {
            Scene Create();
            Entity CreateEntity(Scene& scene, const char* tag = "Entity");
            void Update(Scene& scene);
            void Render(Scene& scene);
        }
    }
}
