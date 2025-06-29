#include "ECS/Scene.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "Core/Application.h"
#include "Core/Utils.h"

#include "Graphics/Renderer.h"

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace ECS
    {
        static Camera2D* activeCamera2D = NULL; // Active 2D runtime camera
        static Camera3D* activeCamera3D = NULL; // Active 3D runtime camera

        namespace Scenes
        {
            void DrawAllCircles(Scene& scene);
            void DrawAllSprites(Scene& scene);

            Scene Create()
            {
                Scene scene;
                scene.state = SceneState::Editor;
                ResetEditorCameras(scene);

                return scene;
            }

            Entity CreateEntity(Scene& scene, const char* tag)
            {
                Entity entity = Entities::Create(scene.registry.create(), &scene);
                entity.AddComponent<InternalComponent>(Random::GenerateUUID(), tag);
                entity.AddComponent<TransformComponent>();

                return entity;
            }

            Entity CreateEntity(Scene& scene, UUID id, const char* tag)
            {
                Entity entity = Entities::Create(scene.registry.create(), &scene);
                entity.AddComponent<InternalComponent>(id, tag);
                entity.AddComponent<TransformComponent>();

                return entity;
            }

            void DestroyEntity(Scene& scene, Entity& entity)
            {
                scene.registry.destroy(entity.handle);
                entity.context = NULL;
            }

            void UpdateEditor(Scene& scene)
            {
                activeCamera2D = NULL;
                activeCamera3D = NULL;
                Cameras::UpdateEditor(scene.editorCamera3D);
            }

            void RenderEditor(Scene& scene)
            {
                Renderer::BeginScene2D(scene.editorCamera3D);
                DrawAllCircles(scene);
                DrawAllSprites(scene);
                Renderer::EndScene2D();
            }

            void UpdateRuntime(Scene& scene)
            {
                activeCamera2D = NULL;
                activeCamera3D = NULL;
                auto cameras = scene.registry.group<Camera2DComponent>(entt::get<TransformComponent>);
                auto sprites = scene.registry.group<SpriteRendererComponent>(entt::get<TransformComponent, InternalComponent>);

                for (auto entityID : cameras)
                {
                    auto& cameraComponent = cameras.get<Camera2DComponent>(entityID);

                    if (cameraComponent.isPrimary)
                    {
                        activeCamera2D = &cameraComponent.camera;
                        break;
                    }
                }
            }

            void RenderRuntime(Scene& scene)
            {
                if (activeCamera2D != NULL)
                {
                    Renderer::BeginScene2D(*activeCamera2D);
                    DrawAllCircles(scene);
                    DrawAllSprites(scene);
                    Renderer::EndScene2D();
                }
            }

            void ResetEditorCameras(Scene& scene)
            {
                const ApplicationConfig& config = Application::GetConfig();

                scene.editorCamera2D.target = glm::vec2(0.f);
                scene.editorCamera2D.offset.x = (float)config.virtualWidth / 2.f;
                scene.editorCamera2D.offset.y = (float)config.virtualHeight / 2.f;
                scene.editorCamera2D.rotation = 0.f;
                scene.editorCamera2D.zoom = 0.f;

                scene.editorCamera3D.target = glm::vec3(0.f);
                scene.editorCamera3D.distance = 15.f;
                scene.editorCamera3D.yaw = 0.f;
                scene.editorCamera3D.pitch = 0.f;
                scene.editorCamera3D.fov = 45.f;
            }

            void DrawAllCircles(Scene& scene)
            {
                auto circles = scene.registry.group<CircleRendererComponent>(entt::get<TransformComponent, InternalComponent>);

                for (auto entityID : circles)
                {
                    auto& internal = circles.get<InternalComponent>(entityID);

                    if (!internal.isActive)
                        continue;

                    auto& transform = circles.get<TransformComponent>(entityID);
                    auto& circleRenderer = circles.get<CircleRendererComponent>(entityID);

                    const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, glm::vec2(circleRenderer.radius),
                                                                                 transform.rotation.z, glm::vec2(0.f));
                    Renderer::DrawEntity(transformMatrix, circleRenderer, (s32)entityID);
                }
            }

            void DrawAllSprites(Scene& scene)
            {
                auto sprites = scene.registry.group<SpriteRendererComponent>(entt::get<TransformComponent, InternalComponent>);

                for (auto entityID : sprites)
                {
                    auto& internal = sprites.get<InternalComponent>(entityID);

                    if (!internal.isActive)
                        continue;

                    auto& transform = sprites.get<TransformComponent>(entityID);
                    auto& spriteRenderer = sprites.get<SpriteRendererComponent>(entityID);

                    const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, transform.scale,
                                                                                 transform.rotation.z, spriteRenderer.origin);
                    Renderer::DrawEntity(transformMatrix, spriteRenderer, (s32)entityID);
                }
            }
        }
    }
}
