#include "ECS/Scene.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"

#include "Core/Application.h"
#include "Core/Time.h"
#include "Core/Utils.h"

#include "Graphics/Renderer.h"

#include <box2d/box2d.h>

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

            template <typename T>
            void CopyComponent(entt::registry& dest, entt::registry& source, const std::unordered_map<UUID, entt::entity>& enttMap);

            template <typename T>
            void CopyComponentIfExists(Entity& dest, Entity& source);

            Scene Create()
            {
                Scene scene;
                scene.physicsWorldID = b2_nullWorldId;
                ResetEditorCameras(scene);

                return scene;
            }

            Scene Copy(Scene& scene)
            {
                Scene newScene = Scenes::Create();

                auto& sourceRegistry = scene.registry;
                auto& destRegistry = newScene.registry;
                std::unordered_map<UUID, entt::entity> enttMap;

                auto idView = sourceRegistry.view<InternalComponent>();
                for (auto entityID : idView)
                {
                    const auto& sourceInternal = sourceRegistry.get<InternalComponent>(entityID);
                    const std::string& tag = sourceInternal.tag;
                    const UUID id = sourceInternal.id;

                    Entity newEntity = CreateEntity(newScene, id, tag.c_str());
                    enttMap[id] = newEntity.handle;
                }

                CopyComponent<TransformComponent>(destRegistry, sourceRegistry, enttMap);
                CopyComponent<SpriteRendererComponent>(destRegistry, sourceRegistry, enttMap);
                CopyComponent<CircleRendererComponent>(destRegistry, sourceRegistry, enttMap);
                CopyComponent<Camera2DComponent>(destRegistry, sourceRegistry, enttMap);
                CopyComponent<Rigidbody2DComponent>(destRegistry, sourceRegistry, enttMap);
                CopyComponent<BoxCollider2DComponent>(destRegistry, sourceRegistry, enttMap);

                return newScene;
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

            Entity DuplicateEntity(Scene& scene, Entity& entity)
            {
                const auto& sourceInternal = entity.GetComponent<InternalComponent>();

                Entity newEntity = CreateEntity(scene, sourceInternal.tag.c_str());
                CopyComponentIfExists<TransformComponent>(newEntity, entity);
                CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
                CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
                CopyComponentIfExists<Camera2DComponent>(newEntity, entity);
                CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
                CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);

                return newEntity;
            }

            void DestroyEntity(Scene& scene, Entity& entity)
            {
                scene.registry.destroy(entity.handle);
                entity.context = NULL;
                entity.handle = entt::null;
            }

            void ClearRegistry(Scene& scene)
            {
                auto entities = scene.registry.view<InternalComponent>();

                for (auto entityID : entities)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    DestroyEntity(scene, entity);
                }
            }

            void OnRuntimeStart(Scene& scene)
            {
                scene.physicsWorld = b2DefaultWorldDef();
                scene.physicsWorld.gravity = (b2Vec2){0.f, -9.81f};
                scene.physicsWorldID = b2CreateWorld(&scene.physicsWorld);

                auto rigidbodies = scene.registry.view<Rigidbody2DComponent>();
                for (auto entityID : rigidbodies)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();

                    b2BodyDef bodyDef = b2DefaultBodyDef();
                    bodyDef.type = (b2BodyType)Utils::BodyTypeToB2BodyType(rb2D.type);
                    bodyDef.motionLocks.angularZ = rb2D.hasFixedRotation;
                    bodyDef.position = (b2Vec2){transform.position.x, transform.position.y};
                    bodyDef.rotation = b2MakeRot(glm::radians(transform.rotation.z));

                    b2BodyId body = b2CreateBody(scene.physicsWorldID, &bodyDef);
                    rb2D.runtimeBody = body;

                    if (entity.HasComponent<BoxCollider2DComponent>())
                    {
                        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

                        b2Polygon polygon = b2MakeBox(bc2d.size.x * transform.scale.x, bc2d.size.y * transform.scale.y);

                        b2ShapeDef shapeDef = b2DefaultShapeDef();
                        shapeDef.density = bc2d.density;
                        shapeDef.material = b2DefaultSurfaceMaterial();
                        shapeDef.material.friction = bc2d.friction;
                        shapeDef.material.restitution = bc2d.restitution;

                        b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &polygon);
                        bc2d.runtimeShape = shape;
                    }
                }
            }

            void OnRuntimeStop(Scene& scene)
            {
                auto rigidbodies = scene.registry.view<Rigidbody2DComponent>();
                for (auto entityID : rigidbodies)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

                    b2DestroyShape(bc2d.runtimeShape, true);
                }

                b2DestroyWorld(scene.physicsWorldID);
                scene.physicsWorldID = b2_nullWorldId;
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
                auto rigidbodies = scene.registry.group<Rigidbody2DComponent>(entt::get<TransformComponent>);

                for (auto entityID : cameras)
                {
                    auto& cameraComponent = cameras.get<Camera2DComponent>(entityID);

                    if (cameraComponent.isPrimary)
                    {
                        activeCamera2D = &cameraComponent.camera;
                        break;
                    }
                }

                b2World_Step(scene.physicsWorldID, Time::GetDelta(), 4);
                for (auto entityID : rigidbodies)
                {
                    auto& transform = rigidbodies.get<TransformComponent>(entityID);
                    auto& rb2d = rigidbodies.get<Rigidbody2DComponent>(entityID);

                    b2Vec2 position = b2Body_GetPosition(rb2d.runtimeBody);
                    float rotationRadians = b2Rot_GetAngle(b2Body_GetRotation(rb2d.runtimeBody));
                    transform.position.x = position.x;
                    transform.position.y = position.y;
                    transform.rotation.z = glm::degrees(rotationRadians);
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

            template <typename T>
            void CopyComponent(entt::registry& dest, entt::registry& source, const std::unordered_map<UUID, entt::entity>& enttMap)
            {
                auto view = source.view<T>();
                for (auto entityID : view)
                {
                    const UUID id = source.get<InternalComponent>(entityID).id;

                    ASSERT(enttMap.find(id) != enttMap.end(), "");
                    entt::entity destEnttID = enttMap.at(id);

                    auto& component = source.get<T>(entityID);
                    dest.emplace_or_replace<T>(destEnttID, component);
                }
            }

            template <typename T>
            void CopyComponentIfExists(Entity& dest, Entity& source)
            {
                if (source.HasComponent<T>())
                    dest.AddComponent<T>(source.GetComponent<T>());
            }
        }
    }
}
