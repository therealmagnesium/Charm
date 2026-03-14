#pragma once
#include "ECS/PhysicsWorld.h"
#include "Core/Random.h"
#include "Graphics/Camera.h"

#include <entt/entt.hpp>

#define MAX_SORTING_LAYERS 8

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm::ECS
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
        u64 entityCount = 0;
        Camera2D editorCamera2D;
        EditorCamera3D editorCamera3D;
        entt::registry registry;
        PhysicsWorld physicsWorld;
        PhysicsWorldID physicsWorldID;
        std::vector<Entity> sortingLayers[MAX_SORTING_LAYERS];
    };

    namespace Scenes
    {
        Scene Create();
        Scene Swap(Scene& scene);
        Scene Copy(Scene& scene);

        Entity CreateEntity(Scene& scene, const char* tag = "Entity");
        Entity CreateEntity(Scene& scene, UUID id, const char* tag = "Entity");
        Entity DuplicateEntity(Scene& scene, Entity& entity);
        void DestroyEntity(Scene& scene, Entity& entity);
        void AddEntityToSortingLayer(Scene& scene, Entity& entity, u32 layer);

        void ClearRegistry(Scene& scene);

        void OnRuntimeStart(Scene& scene);
        void OnRuntimeStop(Scene& scene);

        void UpdateEditor(Scene& scene);
        void RenderEditor(Scene& scene, Entity& selectionContext);

        void UpdateRuntime(Scene& scene);
        void RenderRuntime(Scene& scene);

        void ResetEditorCameras(Scene& scene);
        void AlignParentsAndChildren(Scene& scene);

        Entity GetActiveCameraEntity2D();
        Entity GetActiveCameraEntity3D();
        const Camera2D* GetActiveCamera2D();
        const SceneCamera3D* GetActiveCamera3D();
        const EditorCamera* GetActiveEditorCamera();

        void SetActiveCamera2D(Camera2D* camera);
        void SetActiveCamera3D(SceneCamera3D* camera);
    }
}
