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
        namespace Scenes
        {
            Scene Create()
            {
                const ApplicationConfig& config = Application::GetConfig();

                Scene scene;
                scene.editorCamera2D.target = glm::vec2(0.f);
                scene.editorCamera2D.offset.x = (float)config.virtualWidth / 2.f;
                scene.editorCamera2D.offset.y = (float)config.virtualHeight / 2.f;
                scene.editorCamera2D.rotation = 0.f;
                scene.editorCamera2D.zoom = 1.f;

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
            }

            void Update(Scene& scene) {}

            void Render(Scene& scene)
            {
                auto circles = scene.registry.group<CircleRendererComponent>(entt::get<TransformComponent, InternalComponent>);
                auto sprites = scene.registry.group<SpriteRendererComponent>(entt::get<TransformComponent, InternalComponent>);

                Renderer::BeginScene2D(scene.editorCamera2D);
                for (auto entityID : circles)
                {
                    auto& internal = circles.get<InternalComponent>(entityID);

                    if (internal.isActive)
                    {
                        auto& transform = circles.get<TransformComponent>(entityID);
                        auto& circleRenderer = circles.get<CircleRendererComponent>(entityID);

                        Renderer::DrawCirclePro(transform.position, circleRenderer.radius, circleRenderer.thickness,
                                                circleRenderer.fade, circleRenderer.color);
                    }
                }

                for (auto entityID : sprites)
                {
                    auto& internal = sprites.get<InternalComponent>(entityID);

                    if (internal.isActive)
                    {
                        auto& transform = sprites.get<TransformComponent>(entityID);
                        auto& spriteRenderer = sprites.get<SpriteRendererComponent>(entityID);

                        Texture defaultTexture;
                        defaultTexture.id = 0;
                        defaultTexture.width = 64;
                        defaultTexture.height = 64;

                        Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                        Texture validTexture = (texture != NULL) ? *texture : defaultTexture;

                        Renderer::DrawTextureEx(validTexture, transform.position, transform.rotation.z,
                                                transform.scale, spriteRenderer.tint);
                    }
                }
                Renderer::EndScene2D();
            }
        }
    }
}
