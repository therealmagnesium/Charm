#include "ECS/Scene.h"
#include "ECS/Components.h"
#include "ECS/Entity.h"
#include "ECS/ScriptManager.h"

#include "Core/Application.h"
#include "Core/Time.h"
#include "Core/Utils.h"

#include "Graphics/Renderer.h"
#include "Graphics/RenderCommand.h"

#include "Projects/Project.h"

#include <box2d/box2d.h>
#include <glad/glad.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::Projects;

namespace Charm
{
    namespace ECS
    {
        static Camera2D* activeCamera2D = NULL;      // Active 2D runtime camera
        static SceneCamera3D* activeCamera3D = NULL; // Active 3D runtime camera
        static Entity activeCameraEntity2D = Entity_Null;
        static Entity activeCameraEntity3D = Entity_Null;

        namespace Scenes
        {
            void ApplyCircleSortingLayers(Scene& scene);
            void ApplySpriteSortingLayers(Scene& scene);
            void DrawEntitiesPerSortingLayer(Scene& scene, bool isRuntime);
            void DrawMeshesAndLights(Scene& scene, Entity& selectionContext, bool isRuntime);
            void DrawSelectionContextOutline(Entity& selectionContext);
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
                newScene.isDebugRenderingEnabled = scene.isDebugRenderingEnabled;

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
                CopyComponent<Animator2DComponent>(newScene, scene, enttMap);
                CopyComponent<Camera2DComponent>(newScene, scene, enttMap);
                CopyComponent<Rigidbody2DComponent>(newScene, scene, enttMap);
                CopyComponent<BoxCollider2DComponent>(newScene, scene, enttMap);
                CopyComponent<NativeScriptComponent>(newScene, scene, enttMap);

                const Project& project = ProjectManager::GetActive();
                if (project.type == ProjectType::ThreeDimensional)
                {
                    CopyComponent<MeshRendererComponent>(newScene, scene, enttMap);
                    CopyComponent<DirectionalLightComponent>(newScene, scene, enttMap);
                    CopyComponent<Camera3DComponent>(newScene, scene, enttMap);
                }

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
                CopyComponentIfExists<Animator2DComponent>(newEntity, entity);
                CopyComponentIfExists<Camera2DComponent>(newEntity, entity);
                CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
                CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
                CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);

                const Project& project = ProjectManager::GetActive();
                if (project.type == ProjectType::ThreeDimensional)
                {
                    CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
                    CopyComponentIfExists<DirectionalLightComponent>(newEntity, entity);
                    CopyComponentIfExists<Camera3DComponent>(newEntity, entity);
                }

                Entity activeCameraEntity = Scenes::GetActiveCameraEntity2D();
                if (activeCameraEntity2D != Entity_Null && newEntity.HasComponent<Camera2DComponent>())
                {
                    auto& newCameraComponent = newEntity.GetComponent<Camera2DComponent>();
                    newCameraComponent.isPrimary = false;
                }

                auto& destInternal = newEntity.GetComponent<InternalComponent>();
                destInternal.id = Random::GenerateUUID();

                for (auto& child : Entities::GetChildEntities(entity))
                {
                    Entity newChild = DuplicateEntity(scene, child);
                    auto& childInternal = newChild.GetComponent<InternalComponent>();
                    childInternal.parent = newEntity;
                }

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
                    auto& internal = entity.GetComponent<InternalComponent>();
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();

                    glm::vec2 position = transform.position;
                    if (internal.parent.IsHandleValid())
                    {
                        auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                        position += glm::vec2(parentTransform.position);
                    }

                    b2BodyDef bodyDef = b2DefaultBodyDef();
                    bodyDef.type = (b2BodyType)Utils::BodyTypeToB2BodyType(rb2D.type);
                    bodyDef.motionLocks.angularZ = rb2D.hasFixedRotation;
                    bodyDef.position = *(b2Vec2*)&position;
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
                    shapeDef.invokeContactCreation = false;
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

                b2DestroyWorld(*(b2WorldId*)&scene.physicsWorldID);
                scene.physicsWorldID = Physics_NullWorldID;

                Scenes::ClearRegistry(scene);
            }

            void UpdateEditor(Scene& scene)
            {
                const Project& project = ProjectManager::GetActive();

                for (u8 i = 0; i < LEN(scene.sortingLayers); i++)
                    scene.sortingLayers[i].clear();

                activeCamera2D = NULL;
                activeCamera3D = NULL;
                activeCameraEntity2D = Entity_Null;
                activeCameraEntity3D = Entity_Null;

                if (project.type == ProjectType::TwoDimensional)
                    Cameras::UpdateEditor(scene.editorCamera2D);
                else
                    Cameras::UpdateEditor(scene.editorCamera3D);

                auto sprites = scene.registry.view<SpriteRendererComponent>();
                for (auto entityID : sprites)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                    if (AssetManager::IsHandleValid(spriteRenderer.sprite))
                    {
                        Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                        if (texture->mode == TextureMode::Single || !entity.HasComponent<Animator2DComponent>())
                            continue;

                        const auto& animator = entity.GetComponent<Animator2DComponent>();
                        if (animator.controller == AssetHandle_Invalid)
                            continue;

                        AnimationController* controller = AssetManager::GetAsset<AnimationController>(animator.controller);
                        if (controller->animations.size() < 1)
                            continue;

                        if (controller->animations[0] == AssetHandle_Invalid)
                            continue;

                        Animation* firstAnimation = AssetManager::GetAsset<Animation>(controller->animations[0]);
                        if (firstAnimation->frames.size() < 1)
                            continue;

                        if (spriteRenderer.crop.width < 0.1f || spriteRenderer.crop.height < 0.1f)
                            spriteRenderer.crop = firstAnimation->frames[0];
                    }
                }

                auto cameras = scene.registry.view<Camera2DComponent>();
                for (auto entityID : cameras)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    const auto& cameraComponent = entity.GetComponent<Camera2DComponent>();
                    if (cameraComponent.isPrimary)
                    {
                        activeCameraEntity2D = entity;
                        break;
                    }
                }

                if (project.type == ProjectType::TwoDimensional)
                    return;

                const auto cameras3D = scene.registry.view<Camera3DComponent>();
                for (const auto entityID : cameras3D)
                {
                    Entity entity = Entities::Create(entityID, &scene);

                    const auto& transform = entity.GetComponent<TransformComponent>();
                    auto& cc = entity.GetComponent<Camera3DComponent>();

                    cc.camera.position = transform.position;
                    cc.camera.rotation = transform.rotation;

                    if (cc.isPrimary)
                        activeCameraEntity3D = entity;
                }
            }

            void RenderEditor(Scene& scene, Entity& selectionContext)
            {
                const auto& project = ProjectManager::GetActive();
                if (project.type == ProjectType::ThreeDimensional)
                    DrawMeshesAndLights(scene, selectionContext, false);

                ApplyCircleSortingLayers(scene);
                ApplySpriteSortingLayers(scene);
                DrawEntitiesPerSortingLayer(scene, false);

                if (selectionContext)
                {
                    const auto& transform = selectionContext.GetComponent<TransformComponent>();
                    const glm::vec3 selectionColor = glm::vec3(0.988f, 0.408f, 0.137f);

                    auto& internal = selectionContext.GetComponent<InternalComponent>();

                    if (selectionContext.HasComponent<SpriteRendererComponent>())
                    {
                        const auto& spriteRenderer = selectionContext.GetComponent<SpriteRendererComponent>();
                        glm::mat4 transformMatrix = glm::mat4(1.f);
                        glm::vec2 size = transform.scale;

                        if (AssetManager::IsHandleValid(spriteRenderer.sprite))
                        {
                            Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                            if (texture->mode != TextureMode::Single)
                            {
                                size.x *= (spriteRenderer.crop.width / texture->pixelsPerUnit);
                                size.y *= (spriteRenderer.crop.height / texture->pixelsPerUnit);
                            }
                            else
                            {
                                size.x *= texture->width / (float)texture->pixelsPerUnit;
                                size.y *= texture->height / (float)texture->pixelsPerUnit;
                            }
                        }

                        if (!internal.parent)
                            transformMatrix = Utils::GetTransformMatrix2D(transform.position, size, transform.rotation.z, spriteRenderer.origin);
                        else
                        {
                            auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                            transformMatrix = Utils::GetTransformMatrix2D(transform.position + parentTransform.position, size, transform.rotation.z, spriteRenderer.origin);
                        }

                        Renderer::DrawRectangleLines(transformMatrix, selectionColor);
                    }

                    if (selectionContext.HasComponent<CircleRendererComponent>())
                    {
                        const auto& circleRenderer = selectionContext.GetComponent<CircleRendererComponent>();
                        const glm::mat4 transformMatrix = Utils::GetTransformMatrix2D(transform.position, transform.scale, transform.rotation.z, glm::vec2(0.f));

                        float thickness = GetHighlightThickness(circleRenderer.radius);
                        Renderer::DrawCirclePro(transform.position, circleRenderer.radius, thickness, 0.01f, selectionColor);
                    }
                }
            }

            void UpdateRuntime(Scene& scene)
            {
                activeCamera2D = NULL;
                activeCamera3D = NULL;
                activeCameraEntity2D = Entity_Null;
                activeCameraEntity3D = Entity_Null;

                const auto animators = scene.registry.view<Animator2DComponent>();
                const auto boxColliders = scene.registry.view<BoxCollider2DComponent>();
                const auto cameras = scene.registry.view<Camera2DComponent>();
                const auto nativeScripts = scene.registry.view<NativeScriptComponent>();
                const auto rigidbodies = scene.registry.view<Rigidbody2DComponent>();
                const auto spriteRenderers = scene.registry.view<Animator2DComponent>();

                std::vector<Animation*> updatedAnimations;
                updatedAnimations.reserve(scene.entityCount / 2);

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

                    if (nsc.scriptInstance == NULL)
                        continue;

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

                for (auto entityID : animators)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& animator2D = entity.GetComponent<Animator2DComponent>();
                    AnimationController* controller = AssetManager::GetAsset<AnimationController>(animator2D.controller);
                    Animation* animation = NULL;
                    AssetHandle animationHandle = AssetHandle_Invalid;

                    if (controller == NULL)
                        continue;

                    if (animator2D.activeSlot < 0 || controller->animations.size() < 1 || animator2D.activeSlot > controller->animations.size() - 1)
                        continue;

                    animationHandle = controller->animations[animator2D.activeSlot];
                    animation = AssetManager::GetAsset<Animation>(animationHandle);
                    if (animation == NULL)
                        continue;

                    auto it = std::find(updatedAnimations.begin(), updatedAnimations.end(), animation);
                    if (it == updatedAnimations.end())
                    {
                        Animations::Update(*animation);
                        updatedAnimations.emplace_back(animation);
                    }
                }

                for (auto entityID : animators)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& animator2D = entity.GetComponent<Animator2DComponent>();
                    AnimationController* controller = AssetManager::GetAsset<AnimationController>(animator2D.controller);
                    Animation* animation = NULL;
                    AssetHandle animationHandle = AssetHandle_Invalid;

                    if (controller == NULL)
                        continue;

                    if (animator2D.activeSlot < 0 || controller->animations.size() < 1 || animator2D.activeSlot > controller->animations.size() - 1)
                        continue;

                    animationHandle = controller->animations[animator2D.activeSlot];
                    animation = AssetManager::GetAsset<Animation>(animationHandle);
                    if (animation == NULL || !entity.HasComponent<SpriteRendererComponent>())
                        continue;

                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                    Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);

                    if (texture == NULL)
                        continue;

                    Animations::Apply(*animation, spriteRenderer.crop, *texture);
                }

                for (auto entityID : cameras)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& internal = entity.GetComponent<InternalComponent>();
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& cameraComponent = entity.GetComponent<Camera2DComponent>();

                    cameraComponent.camera.target = transform.position;
                    cameraComponent.camera.rotation = transform.rotation.z;

                    if (internal.parent.IsHandleValid())
                    {
                        auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                        cameraComponent.camera.target += glm::vec2(parentTransform.position);
                    }

                    if (cameraComponent.isPrimary)
                    {
                        activeCamera2D = &cameraComponent.camera;
                        activeCameraEntity2D = entity;
                        break;
                    }
                }

                for (auto entityID : spriteRenderers)
                {
                    Entity entity = Entities::Create(entityID, &scene);
                    auto& transform = entity.GetComponent<TransformComponent>();
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                    spriteRenderer.origin = Utils::OriginModeToVec2(spriteRenderer.originMode, transform.position, transform.scale);
                }

                const Project& project = ProjectManager::GetActive();
                if (project.type == ProjectType::ThreeDimensional)
                {
                    const auto cameras3D = scene.registry.view<Camera3DComponent>();
                    for (const auto entityID : cameras3D)
                    {
                        Entity entity = Entities::Create(entityID, &scene);
                        const auto& transform = entity.GetComponent<TransformComponent>();
                        auto& cc = entity.GetComponent<Camera3DComponent>();

                        cc.camera.position = transform.position;
                        cc.camera.rotation = transform.rotation;

                        if (cc.isPrimary)
                        {
                            activeCamera3D = &cc.camera;
                            activeCameraEntity3D = entity;
                        }
                    }
                }
            }

            void RenderRuntime(Scene& scene)
            {
                const Project& project = ProjectManager::GetActive();
                if (activeCamera2D != NULL && project.type == ProjectType::TwoDimensional)
                {
                    Renderer::BeginScene2D(*activeCamera2D);
                    ApplyCircleSortingLayers(scene);
                    ApplySpriteSortingLayers(scene);
                    DrawEntitiesPerSortingLayer(scene, true);
                    Renderer::EndScene2D();
                }

                if (activeCamera3D != NULL && project.type == ProjectType::ThreeDimensional)
                {
                    Renderer::BeginScene3D(*activeCamera3D);
                    DrawMeshesAndLights(scene, (Entity&)Entity_Null, true);
                    ApplyCircleSortingLayers(scene);
                    ApplySpriteSortingLayers(scene);
                    DrawEntitiesPerSortingLayer(scene, true);
                    Renderer::EndScene2D();
                }
            }

            void ResetEditorCameras(Scene& scene)
            {
                const ApplicationConfig& config = Application::GetConfig();

                scene.editorCamera2D.target = glm::vec2(0.f);
                scene.editorCamera2D.offset.x = 0.f;
                scene.editorCamera2D.offset.y = 0.f;
                scene.editorCamera2D.rotation = 0.f;
                scene.editorCamera2D.zoom = 1.f;

                scene.editorCamera3D.target = glm::vec3(0.f);
                scene.editorCamera3D.distance = 12.f;
                scene.editorCamera3D.yaw = 0.f;
                scene.editorCamera3D.pitch = 25.f;
                scene.editorCamera3D.fov = 45.f;
                scene.editorCamera3D.nearClip = 0.1f;
                scene.editorCamera3D.farClip = 1000.f;
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

            void DrawEntitiesPerSortingLayer(Scene& scene, bool isRuntime)
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

                        if (!internal.isActive)
                            continue;

                        if (entity.HasComponent<CircleRendererComponent>())
                        {
                            const auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();
                            const glm::mat4 transformMatrix = Utils::GetTransformMatrix2D(transform.position, glm::vec2(circleRenderer.radius),
                                                                                          transform.rotation.z, glm::vec2(0.f));
                            Renderer::DrawEntity(transformMatrix, circleRenderer, (s32)entity.handle);
                        }

                        if (entity.HasComponent<SpriteRendererComponent>())
                        {
                            auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                            glm::vec2 size = transform.scale;

                            if (AssetManager::IsHandleValid(spriteRenderer.sprite))
                            {
                                Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
                                if (texture->mode != TextureMode::Single)
                                {
                                    size.x *= spriteRenderer.crop.width / (float)texture->pixelsPerUnit;
                                    size.y *= spriteRenderer.crop.height / (float)texture->pixelsPerUnit;
                                }
                                else
                                {
                                    size.x *= texture->width / (float)texture->pixelsPerUnit;
                                    size.y *= texture->height / (float)texture->pixelsPerUnit;
                                }
                            }

                            spriteRenderer.origin = Utils::OriginModeToVec2(spriteRenderer.originMode, transform.position, size);
                            glm::mat4 transformMatrix = Utils::GetTransformMatrix2D(transform.position, size,
                                                                                    transform.rotation.z, spriteRenderer.origin);
                            if (internal.parent.IsHandleValid())
                            {
                                const auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                                spriteRenderer.origin = Utils::OriginModeToVec2(spriteRenderer.originMode, transform.position + parentTransform.position, size);
                                const glm::mat4 newTransformMatrix = Utils::GetTransformMatrix2D(transform.position + parentTransform.position, size,
                                                                                                 transform.rotation.z, spriteRenderer.origin);

                                if (entity.HasComponent<Rigidbody2DComponent>() && isRuntime)
                                {
                                    const auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
                                    if (rb2D.type != PhysicsBodyType::Static)
                                        transformMatrix = newTransformMatrix;
                                }
                                else
                                    transformMatrix = newTransformMatrix;
                            }

                            Renderer::DrawEntity(transformMatrix, spriteRenderer, (s32)entity.handle);

                            if (entity.HasComponent<BoxCollider2DComponent>() && scene.isDebugRenderingEnabled)
                            {
                                const auto& bc2D = entity.GetComponent<BoxCollider2DComponent>();

                                glm::vec2 origin;
                                origin.x = bc2D.offset.x + bc2D.size.x;
                                origin.y = bc2D.offset.y + bc2D.size.y;

                                glm::mat4 colliderTransformMatrix = Utils::GetTransformMatrix2D(transform.position, bc2D.size * 2.f,
                                                                                                transform.rotation.z, origin);

                                if (internal.parent.IsHandleValid())
                                {
                                    const auto& parentTransform = internal.parent.GetComponent<TransformComponent>();
                                    const glm::mat4 newTransformMatrix = Utils::GetTransformMatrix2D(transform.position + parentTransform.position, bc2D.size * 2.f,
                                                                                                     transform.rotation.z, origin);

                                    if (entity.HasComponent<Rigidbody2DComponent>() && isRuntime)
                                    {
                                        const auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
                                        if (rb2D.type != PhysicsBodyType::Static)
                                            colliderTransformMatrix = newTransformMatrix;
                                    }
                                    else
                                        colliderTransformMatrix = newTransformMatrix;
                                }

                                Renderer::DrawRectangleLines(colliderTransformMatrix, glm::vec3(0.f, 1.f, 0.f));
                                Renderer::DrawCircle(glm::vec2(transform.position), 0.5f, glm::vec3(0.f, 0.f, 1.f));
                            }
                        }
                    }
                }
            }

            void DrawMeshesAndLights(Scene& scene, Entity& selectionContext, bool isRuntime)
            {
                const auto meshes = scene.registry.view<MeshRendererComponent>();

                for (const auto entityID : meshes)
                {
                    Entity entity = Entities::Create(entityID, &scene);

                    const auto& internal = entity.GetComponent<InternalComponent>();
                    if (!internal.isActive)
                        continue;

                    const auto& transform = entity.GetComponent<TransformComponent>();
                    const auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();

                    if (!AssetManager::IsHandleValid(meshRenderer.model))
                        continue;

                    if (entity == selectionContext)
                    {
                        RenderCommand::SetStencilFunc(BufferFunc::Always, 1, 0xFF);
                        RenderCommand::EnableStencilWriting();
                    }
                    else
                    {
                        RenderCommand::SetStencilFunc(BufferFunc::Always, 0, 0xFF);
                        RenderCommand::DisableStencilWriting();
                    }

                    const glm::mat4 transformMatrix = transform.GetMatrix3D();
                    Shader& blinnPhongShader = Renderer::GetShaderBlinnPhong();
                    Model* model = AssetManager::GetAsset<Model>(meshRenderer.model);
                    Renderer::DrawModel(*model, transformMatrix, blinnPhongShader, (s32)entityID);
                }

                if (!isRuntime)
                    DrawSelectionContextOutline(selectionContext);

                const auto suns = scene.registry.view<DirectionalLightComponent>();
                for (const auto entityID : suns)
                {
                    Entity entity = Entities::Create(entityID, &scene);

                    const auto& internal = entity.GetComponent<InternalComponent>();
                    if (!internal.isActive)
                        continue;

                    const auto& dlc = entity.GetComponent<DirectionalLightComponent>();
                    Lights::UpdateUniforms(dlc.sun);
                }
            }

            void DrawSelectionContextOutline(Entity& selectionContext)
            {
                if (selectionContext.IsHandleValid())
                {
                    if (!selectionContext.HasComponent<MeshRendererComponent>())
                        return;

                    auto& transform = selectionContext.GetComponent<TransformComponent>();
                    const auto& meshRenderer = selectionContext.GetComponent<MeshRendererComponent>();
                    const float scale = 1.1f;
                    transform.scale *= scale;

                    if (AssetManager::IsHandleValid(meshRenderer.model))
                    {
                        const glm::mat4 transformMatrix = transform.GetMatrix3D();
                        Shader& outlineShader = Renderer::GetShaderOutline();
                        Model* model = AssetManager::GetAsset<Model>(meshRenderer.model);

                        RenderCommand::SetStencilFunc(BufferFunc::NotEqual, 1, 0xFF);
                        RenderCommand::DisableStencilWriting();
                        RenderCommand::DisableDepthBuffer();
                        Renderer::DrawModel(*model, transformMatrix, outlineShader, (s32)selectionContext.handle);
                        RenderCommand::EnableStencilWriting();
                        RenderCommand::SetStencilFunc(BufferFunc::Always, 0, 0xFF);
                        RenderCommand::EnableDepthBuffer();
                    }
                    transform.scale /= scale;
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

            Entity GetActiveCameraEntity2D() { return activeCameraEntity2D; }
            Entity GetActiveCameraEntity3D() { return activeCameraEntity3D; }
            const Camera2D* GetActiveCamera2D() { return (activeCamera2D != NULL) ? activeCamera2D : &Camera2D_Null; }
            const SceneCamera3D* GetActiveCamera3D() { return (activeCamera3D != NULL) ? activeCamera3D : &SceneCamera3D_Null; }

            void SetActiveCamera2D(Camera2D* camera) { activeCamera2D = camera; }
            void SetActiveCamera3D(SceneCamera3D* camera) { activeCamera3D = camera; }
        }
    }
}
