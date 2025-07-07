#pragma once
#include "Core/Random.h"
#include "Graphics/Camera.h"

#include <box2d/types.h>
#include <entt/entt.hpp>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace ECS
    {
        struct Entity;

        enum class SceneState : u8
        {
            Editor = 0,
            Runtime
        };

        struct Scene
        {
            bool isDebugRenderingEnabled = false;
            entt::registry registry;
            Camera2D editorCamera2D;
            Camera3D editorCamera3D;
            b2WorldDef physicsWorld;
            b2WorldId physicsWorldID;
        };

        namespace Scenes
        {
            Scene Create();
            Scene Copy(Scene& scene);

            Entity CreateEntity(Scene& scene, const char* tag = "Entity");
            Entity CreateEntity(Scene& scene, UUID id, const char* tag = "Entity");
            Entity DuplicateEntity(Scene& scene, Entity& entity);

            void DestroyEntity(Scene& scene, Entity& entity);
            void ClearRegistry(Scene& scene);

            void OnRuntimeStart(Scene& scene);
            void OnRuntimeStop(Scene& scene);

            void UpdateEditor(Scene& scene);
            void RenderEditor(Scene& scene, Entity& selectionContext);

            void UpdateRuntime(Scene& scene);
            void RenderRuntime(Scene& scene);

            void ResetEditorCameras(Scene& scene);
        }
    }
}
