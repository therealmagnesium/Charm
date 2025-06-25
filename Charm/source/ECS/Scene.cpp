#include "ECS/Scene.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "Core/Application.h"

#include "Graphics/Renderer.h"
#include "Graphics/Texture.h"

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace ECS
    {
        static Camera2D* activeCamera2D = NULL;
        static Camera3D* activeCamera3D = NULL;

        namespace Scenes
        {
            void DrawAllCircles(Scene& scene, bool isEditor);
            void DrawAllSprites(Scene& scene, bool isEditor);

            Scene Create()
            {
                const ApplicationConfig& config = Application::GetConfig();

                Scene scene;
                scene.editorCamera2D.target = glm::vec2(0.f);
                scene.editorCamera2D.offset.x = (float)config.virtualWidth / 2.f;
                scene.editorCamera2D.offset.y = (float)config.virtualHeight / 2.f;

                scene.editorCamera3D.target = glm::vec3(0.f);
                scene.editorCamera3D.distance = 15.f;

                return scene;
            }

            Entity CreateEntity(Scene& scene, const char* tag)
            {
                Entity entity = Entities::Create(scene.registry.create(), &scene);
                entity.AddComponent<InternalComponent>(Random::GenerateUUID(), tag);
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
                Cameras::UpdateEditor(scene.editorCamera3D);
            }

            void RenderEditor(Scene& scene)
            {
                Renderer::BeginScene2D(scene.editorCamera3D);
                DrawAllCircles(scene, true);
                DrawAllSprites(scene, true);
                Renderer::EndScene2D();
            }

            void UpdateRuntime(Scene& scene)
            {
                activeCamera2D = NULL;
                auto cameras = scene.registry.group<Camera2DComponent>(entt::get<TransformComponent>);

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
                    DrawAllCircles(scene, false);
                    DrawAllSprites(scene, false);
                    Renderer::EndScene2D();
                }
            }

            void DrawAllCircles(Scene& scene, bool isEditor)
            {
                auto circles = scene.registry.group<CircleRendererComponent>(entt::get<TransformComponent, InternalComponent>);

                for (auto entityID : circles)
                {
                    auto& internal = circles.get<InternalComponent>(entityID);

                    if (internal.isActive)
                    {
                        auto& transform = circles.get<TransformComponent>(entityID);
                        auto& circleRenderer = circles.get<CircleRendererComponent>(entityID);

                        glm::vec2 position = transform.position;
                        float radius = circleRenderer.radius;

                        if (!isEditor)
                        {
                            position.x *= 64.f;
                            position.y *= -64.f;
                            radius *= 64.f;
                        }

                        Renderer::DrawCirclePro(position, radius, circleRenderer.thickness,
                                                circleRenderer.fade, circleRenderer.color);
                    }
                }
            }

            void DrawAllSprites(Scene& scene, bool isEditor)
            {
                auto sprites = scene.registry.group<SpriteRendererComponent>(entt::get<TransformComponent, InternalComponent>);

                for (auto entityID : sprites)
                {
                    auto& internal = sprites.get<InternalComponent>(entityID);

                    if (internal.isActive)
                    {
                        auto& transform = sprites.get<TransformComponent>(entityID);
                        auto& spriteRenderer = sprites.get<SpriteRendererComponent>(entityID);

                        Texture defaultTexture;
                        defaultTexture.id = 0;
                        defaultTexture.width = 1.f;
                        defaultTexture.height = 1.f;

                        glm::vec2 position = transform.position;
                        if (!isEditor)
                        {
                            defaultTexture.width *= 64.f;
                            defaultTexture.height *= 64.f;
                            position.x *= 64.f;
                            position.y *= -64.f;
                        }

                        Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                        Texture validTexture = (texture != NULL) ? *texture : defaultTexture;

                        Renderer::DrawTextureEx(validTexture, position, transform.rotation.z,
                                                transform.scale, spriteRenderer.tint);
                    }
                }
            }
        }
    }
}
