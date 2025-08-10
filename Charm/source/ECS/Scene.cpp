#include "ECS/Scene.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ScriptManager.h"

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
            void ApplyCircleSortingLayers(Scene& scene);
            void ApplySpriteSortingLayers(Scene& scene);
            void DrawEntitiesPerSortingLayer(Scene& scene);
            float GetHighlightThickness(float radius);

            template <typename T>
            void CopyComponent(Scene& dest, Scene& source, const std::unordered_map<UUID, entt::entity>& enttMap);

            template <typename T>
            void CopyComponentIfExists(Entity& dest, Entity& source);

            Scene Create()
            {
                Scene scene;
                scene.physicsWorldID = Physics_NullWorldID;

                for (u8 i = 0; i < LEN(scene.sortingLayers); i++)
                    scene.sortingLayers[i].reserve(1);

                Scenes::ResetEditorCameras(scene);

                return scene;
            }

            Scene Swap(Scene& scene)
            {
                Scene newScene = Scenes::Create();

                auto& sourceRegistry = scene.registry;
                auto& destRegistry = newScene.registry;

                sourceRegistry.swap(destRegistry);

                return newScene;
            }

            Scene Copy(Scene& scene)
            {
                Scene newScene = Scenes::Create();

                auto& sourceRegistry = scene.registry;
                auto& destRegistry = newScene.registry;
                std::unordered_map<UUID, entt::entity> enttMap;

                auto entities = sourceRegistry.view<InternalComponent>();
                for (auto entityID : entities)
                {
                    const auto& sourceInternal = sourceRegistry.get<InternalComponent>(entityID);
                    const std::string& tag = sourceInternal.tag;
                    const UUID id = sourceInternal.id;

                    Entity newEntity = CreateEntity(newScene, id, tag.c_str());
                    enttMap[id] = newEntity.handle;
                }

                CopyComponent<InternalComponent>(newScene, scene, enttMap);
                CopyComponent<TransformComponent>(newScene, scene, enttMap);
                CopyComponent<SpriteRendererComponent>(newScene, scene, enttMap);
                CopyComponent<CircleRendererComponent>(newScene, scene, enttMap);
                CopyComponent<Camera2DComponent>(newScene, scene, enttMap);
                CopyComponent<Rigidbody2DComponent>(newScene, scene, enttMap);
                CopyComponent<BoxCollider2DComponent>(newScene, scene, enttMap);
                CopyComponent<NativeScriptComponent>(newScene, scene, enttMap);

                return newScene;
            }

            Entity CreateEntity(Scene& scene, const char* tag)
            {
                Entity entity = Entities::Create(scene.registry.create(), &scene);
                entity.AddComponent<InternalComponent>(Random::GenerateUUID(), tag);
                entity.AddComponent<TransformComponent>();

                scene.entityCount++;
                return entity;
            }

            Entity CreateEntity(Scene& scene, UUID id, const char* tag)
            {
                Entity entity = Entities::Create(scene.registry.create(), &scene);
                entity.AddComponent<InternalComponent>(id, tag);
                entity.AddComponent<TransformComponent>();

                scene.entityCount++;
                return entity;
            }

            Entity DuplicateEntity(Scene& scene, Entity& entity)
            {
                const auto& sourceInternal = entity.GetComponent<InternalComponent>();

                Entity newEntity = CreateEntity(scene, sourceInternal.tag.c_str());
                CopyComponentIfExists<InternalComponent>(newEntity, entity);
                CopyComponentIfExists<TransformComponent>(newEntity, entity);
                CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
                CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
                CopyComponentIfExists<Camera2DComponent>(newEntity, entity);
                CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
                CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
                CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);

                auto& destInternal = newEntity.GetComponent<InternalComponent>();
                destInternal.id = Random::GenerateUUID();

                return newEntity;
            }

            void DestroyEntity(Scene& scene, Entity& entity)
            {
                scene.registry.destroy(entity.handle);
                entity.context = NULL;
                entity.handle = entt::null;
                scene.entityCount--;
            }

            void AddEntityToSortingLayer(Scene& scene, Entity& entity, u32 layer)
            {
                std::vector<Entity>& sortingLayer = scene.sortingLayers[layer];

                if (entity && (std::find(sortingLayer.begin(), sortingLayer.end(), entity) == sortingLayer.end()))
                    scene.sortingLayers[layer].emplace_back(entity);
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
                scene.entityCount = 0;

                scene = Scenes::Create();
            }

            void OnRuntimeStart(Scene& scene)
            {
                auto nativeScripts = scene.registry.view<NativeScriptComponent>();
                for (auto entityID : nativeScripts)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& nsc = entity.GetComponent<NativeScriptComponent>();

                    nsc.CreateScript = ScriptManager::GetScriptInitFunc(nsc.scriptName);
                    if (nsc.scriptInstance == NULL && nsc.CreateScript != NULL)
                    {
                        nsc.scriptInstance = nsc.CreateScript();
                        nsc.scriptInstance->m_entity = entity;
                        nsc.scriptInstance->OnCreate();
                    }
                }

                b2WorldDef worldDef = b2DefaultWorldDef();
                scene.physicsWorld = Utils::B2WorldDefToPhysicsWorld(worldDef);
                scene.physicsWorld.gravity = (PhysicsWorld::Vec2){0.f, -9.81f};

                b2WorldId worldID = b2CreateWorld((b2WorldDef*)&scene.physicsWorld);
                scene.physicsWorldID = Utils::B2WorldToPhysicsWorldID(worldID);

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
                    bodyDef.linearDamping = rb2D.linearDamping;
                    bodyDef.angularDamping = rb2D.angularDamping;
                    bodyDef.gravityScale = rb2D.gravityScale;
                    bodyDef.isBullet = false;

                    b2BodyId body = b2CreateBody(*(b2WorldId*)&scene.physicsWorldID, &bodyDef);
                    rb2D.runtimeBody = Utils::B2BodyToPhysicsBody(body);
                }

                auto boxColliders = scene.registry.view<BoxCollider2DComponent>();
                for (auto entityID : boxColliders)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                    b2Vec2 center = (b2Vec2){bc2D.offset.x, -bc2D.offset.y};
                    b2Polygon polygon = b2MakeOffsetBox(bc2D.size.x, bc2D.size.y, center, b2MakeRot(0.f));

                    b2ShapeDef shapeDef = b2DefaultShapeDef();
                    shapeDef.isSensor = bc2D.isTrigger;
                    shapeDef.density = bc2D.density;
                    shapeDef.enableContactEvents = true;
                    shapeDef.material = b2DefaultSurfaceMaterial();
                    shapeDef.material.friction = bc2D.friction;
                    shapeDef.material.restitution = bc2D.restitution;

                    b2BodyId body{};
                    if (entity.HasComponent<Rigidbody2DComponent>())
                    {
                        auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
                        body = *(b2BodyId*)&rb2D.runtimeBody;
                    }

                    b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &polygon);
                    bc2D.runtimeShape = Utils::B2ShapeToPhysicsShape(shape);
                }
            }

            void OnRuntimeStop(Scene& scene)
            {
                auto nativeScripts = scene.registry.view<NativeScriptComponent>();
                for (auto entityID : nativeScripts)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& nsc = entity.GetComponent<NativeScriptComponent>();
                    nsc.DestroyScript = ScriptManager::GetScriptDestroyFunc(nsc.scriptName);

                    if (nsc.scriptInstance != NULL && nsc.DestroyScript != NULL)
                    {
                        nsc.scriptInstance->OnDestroy();
                        nsc.DestroyScript(nsc.scriptInstance);
                    }
                }

                auto rigidbodies = scene.registry.view<Rigidbody2DComponent>();
                for (auto entityID : rigidbodies)
                {
                    Entity entity = Entities::Create(entityID, &scene);

                    if (entity.HasComponent<BoxCollider2DComponent>())
                    {
                        auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                        b2DestroyShape(*(b2ShapeId*)&bc2d.runtimeShape, true);
                    }
                }

                b2DestroyWorld(*(b2WorldId*)&scene.physicsWorldID);
                scene.physicsWorldID = Physics_NullWorldID;

                Scenes::ClearRegistry(scene);
            }

            void UpdateEditor(Scene& scene)
            {
                for (u8 i = 0; i < LEN(scene.sortingLayers); i++)
                    scene.sortingLayers[i].clear();

                activeCamera2D = NULL;
                activeCamera3D = NULL;
                Cameras::UpdateEditor(scene.editorCamera3D);

                auto sprites = scene.registry.view<SpriteRendererComponent>();
                for (auto entityID : sprites)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                    spriteRenderer.origin = Utils::OriginModeToVec2(spriteRenderer.originMode, transform.position, transform.scale);
                }
            }

            void RenderEditor(Scene& scene, Entity& selectionContext)
            {
                Renderer::BeginScene2D(scene.editorCamera3D);
                ApplyCircleSortingLayers(scene);
                ApplySpriteSortingLayers(scene);
                DrawEntitiesPerSortingLayer(scene);

                if (selectionContext)
                {
                    const auto& transform = selectionContext.GetComponent<TransformComponent>();
                    const glm::vec3 selectionColor = glm::vec3(0.8f, 0.4f, 0.2f);

                    auto& internal = selectionContext.GetComponent<InternalComponent>();

                    if (selectionContext.HasComponent<SpriteRendererComponent>())
                    {
                        const auto& spriteRenderer = selectionContext.GetComponent<SpriteRendererComponent>();
                        glm::mat4 transformMatrix = glm::mat4(1.f);

                        if (!internal.parent)
                            transformMatrix = Utils::GetTransfomMatrix2D(transform.position, transform.scale, transform.rotation.z, spriteRenderer.origin);
                        else
                        {
                            auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                            transformMatrix = Utils::GetTransfomMatrix2D(transform.position + parentTransform.position, transform.scale, transform.rotation.z, spriteRenderer.origin);
                        }

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
                auto boxColliders = scene.registry.view<BoxCollider2DComponent>();
                auto nativeScripts = scene.registry.view<NativeScriptComponent>();

                b2World_Step(*(b2WorldId*)&scene.physicsWorldID, Time::GetDelta(), 4);
                std::unordered_map<Entity, Entity> contactBeginEntities;
                std::unordered_map<Entity, Entity> contactEndEntities;

                contactBeginEntities.reserve(scene.entityCount);
                contactEndEntities.reserve(scene.entityCount);

                for (auto entityID : rigidbodies)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
                    auto& runtimeBody = *(b2BodyId*)&rb2D.runtimeBody;

                    if (!b2Body_IsValid(runtimeBody))
                        continue;

                    b2Vec2 linearVelocity = b2Body_GetLinearVelocity(runtimeBody);
                    float angularVelocity = b2Body_GetAngularVelocity(runtimeBody);

                    rb2D.linearVelocity = *(glm::vec2*)&linearVelocity;
                    rb2D.angularVelocity = glm::degrees(angularVelocity);

                    b2Body_SetLinearVelocity(runtimeBody, *(b2Vec2*)&rb2D.linearVelocity);
                    b2Body_SetAngularVelocity(runtimeBody, glm::radians(rb2D.angularVelocity));

                    b2Vec2 position = b2Body_GetPosition(runtimeBody);
                    float rotationRadians = b2Rot_GetAngle(b2Body_GetRotation(runtimeBody));
                    transform.position.x = position.x;
                    transform.position.y = position.y;
                    transform.rotation.z = glm::degrees(rotationRadians);
                }

                auto& physicsWorld = *(b2WorldId*)&scene.physicsWorldID;
                b2ContactEvents contactEvents = b2World_GetContactEvents(physicsWorld);

                for (u32 i = 0; i < contactEvents.beginCount; i++)
                {
                    b2ContactBeginTouchEvent& event = contactEvents.beginEvents[i];
                    PhysicsShapeID shapeA = Utils::B2ShapeToPhysicsShape(event.shapeIdA);
                    PhysicsShapeID shapeB = Utils::B2ShapeToPhysicsShape(event.shapeIdB);

                    Entity entityA;
                    Entity entityB;
                    for (auto entityID : boxColliders)
                    {
                        Entity entity = Entities::Create(entityID, &scene);
                        auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                        if (shapeA == bc2D.runtimeShape)
                            entityA = entity;

                        if (shapeB == bc2D.runtimeShape)
                            entityB = entity;

                        if (entityA && entityB)
                        {
                            contactBeginEntities[entityB] = entityA;
                            continue;
                        }
                    }
                }

                for (u32 i = 0; i < contactEvents.endCount; i++)
                {
                    b2ContactEndTouchEvent& event = contactEvents.endEvents[i];
                    PhysicsShapeID shapeA = Utils::B2ShapeToPhysicsShape(event.shapeIdA);
                    PhysicsShapeID shapeB = Utils::B2ShapeToPhysicsShape(event.shapeIdB);

                    if (!b2Shape_IsValid(*(b2ShapeId*)&shapeA) || !b2Shape_IsValid(*(b2ShapeId*)&shapeB))
                        continue;

                    Entity entityA;
                    Entity entityB;
                    for (auto entityID : boxColliders)
                    {
                        Entity entity = Entities::Create(entityID, &scene);
                        auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                        if (shapeA == bc2D.runtimeShape)
                            entityA = entity;

                        if (shapeB == bc2D.runtimeShape)
                            entityB = entity;

                        if (entityA && entityB)
                        {
                            contactEndEntities[entityB] = entityA;
                            continue;
                        }
                    }
                }

                for (auto entityID : nativeScripts)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& nsc = entity.GetComponent<NativeScriptComponent>();

                    if (nsc.scriptInstance != NULL)
                        nsc.scriptInstance->OnUpdate();

                    if (contactBeginEntities.find(entity) != contactBeginEntities.end())
                    {
                        Entity& collidedEntity = contactBeginEntities[entity];
                        nsc.scriptInstance->OnCollisionEnter(collidedEntity);
                    }

                    if (contactEndEntities.find(entity) != contactEndEntities.end())
                    {
                        Entity& collidedEntity = contactEndEntities[entity];
                        nsc.scriptInstance->OnCollisionExit(collidedEntity);
                    }
                }

                for (auto entityID : cameras)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& cameraComponent = entity.GetComponent<Camera2DComponent>();

                    cameraComponent.camera.target = transform.position;
                    cameraComponent.camera.rotation = transform.rotation.z;

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
                    ApplyCircleSortingLayers(scene);
                    ApplySpriteSortingLayers(scene);
                    DrawEntitiesPerSortingLayer(scene);
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

            void AlignParentsAndChildren(Scene& scene)
            {
                auto entities = scene.registry.view<InternalComponent>();
                for (auto entityID : entities)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& internal = entity.GetComponent<InternalComponent>();

                    if (internal.parent)
                    {
                        const UUID parentUUID = internal.parent.GetComponent<InternalComponent>().id;
                        internal.parent = Entities::FindWithUUID(parentUUID, &scene);
                    }
                }
            }

            void ApplyCircleSortingLayers(Scene& scene)
            {
                auto circles = scene.registry.view<CircleRendererComponent>();

                for (auto entityID : circles)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& internal = entity.GetComponent<InternalComponent>();
                    auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();

                    if (!internal.isActive)
                        continue;

                    Scenes::AddEntityToSortingLayer(scene, entity, circleRenderer.sortingLayer);
                }
            }

            void ApplySpriteSortingLayers(Scene& scene)
            {
                auto sprites = scene.registry.view<SpriteRendererComponent>();

                for (auto entityID : sprites)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& internal = entity.GetComponent<InternalComponent>();
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                    if (!internal.isActive)
                        continue;

                    Scenes::AddEntityToSortingLayer(scene, entity, spriteRenderer.sortingLayer);
                }
            }

            void DrawEntitiesPerSortingLayer(Scene& scene)
            {
                for (u32 i = 0; i < LEN(scene.sortingLayers); i++)
                {
                    if (scene.sortingLayers[i].size() < 1)
                        continue;

                    for (Entity& entity : scene.sortingLayers[i])
                    {
                        if (!entity.IsHandleValid())
                            continue;

                        auto& internal = entity.GetComponent<InternalComponent>();
                        auto& transform = entity.GetComponent<TransformComponent>();

                        if (entity.HasComponent<CircleRendererComponent>())
                        {
                            auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();
                            const glm::mat4 transformMatrix = Utils::GetTransfomMatrix2D(transform.position, glm::vec2(circleRenderer.radius),
                                                                                         transform.rotation.z, glm::vec2(0.f));
                            Renderer::DrawEntity(transformMatrix, circleRenderer, (s32)entity.handle);
                        }

                        if (entity.HasComponent<SpriteRendererComponent>())
                        {
                            auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                            if (!internal.isActive)
                                continue;

                            glm::mat4 transformMatrix = glm::mat4(1.f);
                            if (!internal.parent)
                                transformMatrix = Utils::GetTransfomMatrix2D(transform.position, transform.scale,
                                                                             transform.rotation.z, spriteRenderer.origin);
                            else
                            {
                                auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                                transformMatrix = Utils::GetTransfomMatrix2D(transform.position + parentTransform.position, transform.scale,
                                                                             transform.rotation.z, spriteRenderer.origin);
                            }

                            Renderer::DrawEntity(transformMatrix, spriteRenderer, (s32)entity.handle);

                            if (entity.HasComponent<BoxCollider2DComponent>() && scene.isDebugRenderingEnabled)
                            {
                                auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                                glm::vec2 origin;
                                origin.x = bc2D.offset.x + bc2D.size.x;
                                origin.y = bc2D.offset.y + bc2D.size.y;

                                glm::mat4 colliderTransformMatrix = glm::mat4(1.f);

                                if (!internal.parent)
                                    colliderTransformMatrix = Utils::GetTransfomMatrix2D(transform.position, bc2D.size * 2.f,
                                                                                         transform.rotation.z, origin);
                                else
                                {
                                    const auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                                    colliderTransformMatrix = Utils::GetTransfomMatrix2D(transform.position + parentTransform.position, bc2D.size * 2.f,
                                                                                         transform.rotation.z, origin);
                                }

                                Renderer::DrawRectangleLines(colliderTransformMatrix, glm::vec3(0.f, 1.f, 0.f));
                            }
                        }
                    }
                }
            }

            template <typename T>
            void CopyComponent(Scene& dest, Scene& source, const std::unordered_map<UUID, entt::entity>& enttMap)
            {
                auto view = source.registry.view<T>();
                for (auto entityID : view)
                {
                    const UUID id = source.registry.get<InternalComponent>(entityID).id;

                    ASSERT(enttMap.find(id) != enttMap.end(), "Scenes::CopyComponent - Entity map could not find entity with UUID %ld", id);

                    entt::entity destEnttID = enttMap.at(id);
                    Entity sourceEntity = Entities::Create(entityID, &source);
                    Entity destEntity = Entities::Create(destEnttID, &dest);
                    auto& sourceInternal = sourceEntity.GetComponent<InternalComponent>();

                    auto& component = sourceEntity.GetComponent<T>();
                    destEntity.AddComponent<T>(component);
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
