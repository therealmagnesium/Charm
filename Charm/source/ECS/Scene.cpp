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
            float GetHighlightThickness(float radius);

            template <typename T>
            void CopyComponent(entt::registry& dest, entt::registry& source, const std::unordered_map<UUID, entt::entity>& enttMap);

            template <typename T>
            void CopyComponentIfExists(Entity& dest, Entity& source);

            Scene Create()
            {
                Scene scene;
                scene.physicsWorldID = b2_nullWorldId;
                Scenes::ResetEditorCameras(scene);

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

                scene.registry.clear();
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
                        auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                        b2Vec2 center = (b2Vec2){bc2D.offset.x, -bc2D.offset.y};
                        b2Polygon polygon = b2MakeOffsetBox(bc2D.size.x, bc2D.size.y, center, b2MakeRot(0.f));

                        b2ShapeDef shapeDef = b2DefaultShapeDef();
                        shapeDef.density = bc2D.density;
                        shapeDef.material = b2DefaultSurfaceMaterial();
                        shapeDef.material.friction = bc2D.friction;
                        shapeDef.material.restitution = bc2D.restitution;

                        b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &polygon);
                        bc2D.runtimeShape = shape;
                    }
                }
            }

            void OnRuntimeStop(Scene& scene)
            {
                auto rigidbodies = scene.registry.view<Rigidbody2DComponent>();
                for (auto entityID : rigidbodies)
                {
                    Entity entity = Entities::Create(entityID, &scene);

                    if (entity.HasComponent<BoxCollider2DComponent>())
                    {
                        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                        b2DestroyShape(bc2d.runtimeShape, true);
                    }
                }

                b2DestroyWorld(scene.physicsWorldID);
                scene.physicsWorldID = b2_nullWorldId;

                Scenes::ClearRegistry(scene);
            }

            void UpdateEditor(Scene& scene)
            {
                activeCamera2D = NULL;
                activeCamera3D = NULL;
                Cameras::UpdateEditor(scene.editorCamera3D);

                auto sprites = scene.registry.view<SpriteRendererComponent>();
                for (auto entityID : sprites)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                    spriteRenderer.origin = transform.scale / 2.f; // Temp - Eventually have 9 origin modes
                }
            }

            void RenderEditor(Scene& scene, Entity& selectionContext)
            {
                Renderer::BeginScene2D(scene.editorCamera3D);
                DrawAllCircles(scene);
                DrawAllSprites(scene);

                if (selectionContext)
                {
                    const auto& transform = selectionContext.GetComponent<TransformComponent>();
                    const glm::vec3 selectionColor = glm::vec3(0.8f, 0.4f, 0.2f);

                    if (selectionContext.HasComponent<SpriteRendererComponent>())
                    {
                        const auto& spriteRenderer = selectionContext.GetComponent<SpriteRendererComponent>();
                        const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, transform.scale, transform.rotation.z, spriteRenderer.origin);

                        Renderer::DrawRectangleLines(transformMatrix, selectionColor);
                    }

                    if (selectionContext.HasComponent<CircleRendererComponent>())
                    {
                        const auto& circleRenderer = selectionContext.GetComponent<CircleRendererComponent>();
                        const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, transform.scale, transform.rotation.z, glm::vec2(0.f));

                        float thickness = GetHighlightThickness(circleRenderer.radius);
                        Renderer::DrawCirclePro(transform.position, circleRenderer.radius, thickness, 0.01f, selectionColor);
                    }
                }
                Renderer::EndScene2D();
            }

            void UpdateRuntime(Scene& scene)
            {
                activeCamera2D = NULL;
                activeCamera3D = NULL;
                auto cameras = scene.registry.view<Camera2DComponent>();
                auto rigidbodies = scene.registry.view<Rigidbody2DComponent>();

                for (auto entityID : cameras)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& cameraComponent = entity.GetComponent<Camera2DComponent>();

                    if (cameraComponent.isPrimary)
                    {
                        activeCamera2D = &cameraComponent.camera;
                        break;
                    }
                }

                b2World_Step(scene.physicsWorldID, Time::GetDelta(), 4);
                for (auto entityID : rigidbodies)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

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
                auto circles = scene.registry.view<CircleRendererComponent>();

                for (auto entityID : circles)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& internal = entity.GetComponent<InternalComponent>();

                    if (!internal.isActive)
                        continue;

                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();

                    const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, glm::vec2(circleRenderer.radius),
                                                                                 transform.rotation.z, glm::vec2(0.f));
                    Renderer::DrawEntity(transformMatrix, circleRenderer, (s32)entityID);
                }
            }

            void DrawAllSprites(Scene& scene)
            {
                auto sprites = scene.registry.view<SpriteRendererComponent>();

                for (auto entityID : sprites)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& internal = entity.GetComponent<InternalComponent>();

                    if (!internal.isActive)
                        continue;

                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                    const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, transform.scale,
                                                                                 transform.rotation.z, spriteRenderer.origin);

                    Renderer::DrawEntity(transformMatrix, spriteRenderer, (s32)entityID);

                    if (entity.HasComponent<BoxCollider2DComponent>() && scene.isDebugRenderingEnabled)
                    {
                        auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                        glm::vec2 origin;
                        origin.x = bc2D.offset.x + bc2D.size.x;
                        origin.y = bc2D.offset.y + bc2D.size.y;

                        const glm::mat4 colliderTransformMatrix = Utils::GetTransfomMatrix2D(transform.position, bc2D.size * 2.f,
                                                                                             transform.rotation.z, origin);
                        Renderer::DrawRectangleLines(colliderTransformMatrix, glm::vec3(0.f, 1.f, 0.f));
                    }
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

            float GetHighlightThickness(float radius)
            {
                float maxThickness = 0.4f;  // Max cap (fully filled)
                float minThickness = 0.05f; // Minimum visible stroke width
                float scale = 1.0f;         // Controls how quickly the thickness shrinks with larger radius

                float thickness = (scale / radius) / radius;

                // Clamp thickness between minThickness and maxThickness
                if (thickness > maxThickness)
                    thickness = maxThickness;
                if (thickness < minThickness)
                    thickness = minThickness;

                return thickness;
            }
        }
    }
}
