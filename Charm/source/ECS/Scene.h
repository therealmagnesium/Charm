#pragma once
#include "Core/Random.h"
#include "Graphics/Camera.h"

#include <entt/entt.hpp>

using namespace Charm::Core;
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
            Camera3D editorCamera3D;
        };

        namespace Scenes
        {
            Scene Create();

            Entity CreateEntity(Scene& scene, const char* tag = "Entity");
            Entity CreateEntity(Scene& scene, UUID id);
            void DestroyEntity(Scene& scene, Entity& entity);

            void UpdateEditor(Scene& scene);
            void RenderEditor(Scene& scene);

            void UpdateRuntime(Scene& scene);
            void RenderRuntime(Scene& scene);

            void ResetEditorCameras(Scene& scene);
        }
    }
}
