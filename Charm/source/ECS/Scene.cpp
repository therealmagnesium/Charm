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
                Entity entity = Entities::Create(scene.registry.create(), &scene, tag);
                entity.AddComponent<TransformComponent>();

                return entity;
            }

            void Update(Scene& scene) {}

            void Render(Scene& scene)
            {
                auto circles = scene.registry.group<CircleRendererComponent>(entt::get<TransformComponent>);
                auto sprites = scene.registry.group<SpriteRendererComponent>(entt::get<TransformComponent>);

                for (auto entity : circles)
                {
                    auto [transform, circleRenderer] = circles.get<TransformComponent, CircleRendererComponent>(entity);

                    Renderer::DrawCirclePro(transform.position, circleRenderer.radius, circleRenderer.thickness,
                                            circleRenderer.fade, circleRenderer.color);
                }

                for (auto entity : sprites)
                {
                    auto [transform, spriteRenderer] = sprites.get<TransformComponent, SpriteRendererComponent>(entity);

                    Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                    Texture validTexture = (texture != NULL) ? *texture : (Texture){};

                    Renderer::DrawTextureEx(validTexture, transform.position, transform.rotation.z,
                                            transform.scale, spriteRenderer.tint);
                }
            }
        }
    }
}
