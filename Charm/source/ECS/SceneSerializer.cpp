#include "ECS/SceneSerializer.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

#include "Core/Log.h"
#include "Core/Utils.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>

namespace YAML
{
    template <>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& v)
        {
            Node node;
            node.push_back(v.x);
            node.push_back(v.y);

            return node;
        }

        static bool decode(const Node& node, glm::vec2& v)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            v.x = node[0].as<float>();
            v.y = node[1].as<float>();

            return true;
        }
    };

    template <>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& v)
        {
            Node node;
            node.push_back(v.x);
            node.push_back(v.y);
            node.push_back(v.z);

            return node;
        }

        static bool decode(const Node& node, glm::vec3& v)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            v.x = node[0].as<float>();
            v.y = node[1].as<float>();
            v.z = node[2].as<float>();

            return true;
        }
    };

    template <>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& v)
        {
            Node node;
            node.push_back(v.r);
            node.push_back(v.g);
            node.push_back(v.b);
            node.push_back(v.a);

            return node;
        }

        static bool decode(const Node& node, glm::vec4& v)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            v.r = node[0].as<float>();
            v.g = node[1].as<float>();
            v.b = node[2].as<float>();
            v.a = node[3].as<float>();

            return true;
        }
    };

    template <>
    struct convert<Rectangle>
    {
        static Node encode(const Rectangle& r)
        {
            Node node;
            node.push_back(r.x);
            node.push_back(r.y);
            node.push_back(r.width);
            node.push_back(r.height);

            return node;
        }

        static bool decode(const Node& node, Rectangle& r)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            r.x = node[0].as<float>();
            r.y = node[1].as<float>();
            r.width = node[2].as<float>();
            r.height = node[3].as<float>();

            return true;
        }
    };
}

namespace Charm
{
    namespace ECS
    {
        static Scene* context = NULL;

        YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
        {
            out << YAML::Flow;
            out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
            return out;
        }

        YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
        {
            out << YAML::Flow;
            out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
            return out;
        }

        YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
        {
            out << YAML::Flow;
            out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
            return out;
        }

        YAML::Emitter& operator<<(YAML::Emitter& out, const Rectangle& r)
        {
            out << YAML::Flow;
            out << YAML::BeginSeq << r.x << r.y << r.width << r.height << YAML::EndSeq;
            return out;
        }

        namespace SceneSerializer
        {
            void SerializeEntity(YAML::Emitter& out, Entity& entity);
            void DeserializeEntity(Entity& entity, YAML::Node& node);

            void SetContext(Scene& scene) { context = &scene; }

            void Serialize(const char* path)
            {
                ASSERT_ERROR(context != NULL, "SceneSerializer::Serialize - The context has not been set!");

                YAML::Emitter out;
                out << YAML::BeginMap;
                out << YAML::Key << "Scene" << YAML::Value << "Untitled";

                out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
                for (auto entityID : context->registry.view<entt::entity>())
                {
                    Entity entity = Entities::Create(entityID, context);

                    if (!entity)
                        continue;

                    SerializeEntity(out, entity);
                }
                out << YAML::EndSeq;

                out << YAML::Key << "Asset Registry" << YAML::Value << YAML::BeginSeq;
                for (auto& [handle, metadata] : AssetManager::GetRegistry())
                {
                    out << YAML::BeginMap;
                    out << YAML::Key << "Asset" << YAML::Value << handle;
                    out << YAML::Key << "Path" << YAML::Value << metadata.path;
                    out << YAML::Key << "Type" << YAML::Value << Utils::AssetTypeToString(metadata.type);

                    Asset* asset = AssetManager::GetAsset(handle);
                    switch (asset->GetType())
                    {
                        case AssetType::Texture:
                        {
                            Texture* texture = (Texture*)asset;
                            out << YAML::Key << "Texture Filter" << YAML::Value << Utils::TextureFilterToString(texture->filter);
                            break;
                        }

                        default:
                            break;
                    }
                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;

                out << YAML::EndMap;

                std::ofstream fout(path);
                fout << out.c_str() << "\n";
                fout.close();
            }

            void SerializeRuntime(const char* path) { ASSERT(false, "SceneSerializer::SerializeRuntime - Not implemented yet!"); }

            void Deserialize(const char* path)
            {
                ASSERT_ERROR(context != NULL, "SceneSerializer::Deserialize - The context has not been set!");

                std::stringstream stream;
                std::ifstream in(path);
                stream << in.rdbuf();
                in.close();

                YAML::Node data = YAML::Load(stream.str());
                if (!data["Scene"])
                {
                    ERROR("SceneSerializer::Deserialize - Failed to find root scene node in %s!", path);
                    return;
                }

                YAML::Node entities = data["Entities"];
                std::vector<std::pair<UUID, UUID>> parentChildRelationships;

                if (entities)
                {
                    for (auto entity : entities)
                    {
                        UUID uuid = entity["Entity"].as<UUID>();
                        Entity deserializedEntity = Scenes::CreateEntity(*context, uuid);

                        YAML::Node internalNode = entity["Internal Component"];
                        UUID parentUUID = internalNode["Parent"].as<UUID>();
                        if (parentUUID != 0)
                            parentChildRelationships.emplace_back(uuid, parentUUID);

                        DeserializeEntity(deserializedEntity, entity);
                    }

                    for (const auto& [childUUID, parentUUID] : parentChildRelationships)
                    {
                        Entity child = Entities::FindWithUUID(childUUID, context);
                        Entity parent = Entities::FindWithUUID(parentUUID, context);

                        if (!child.IsHandleValid() || !parent.IsHandleValid())
                        {
                            ERROR("SceneSerializer::Deserialize - Failed to establish parent-child relationship between UUIDs %lu and %lu", parentUUID, childUUID);
                            continue;
                        }

                        auto& childInternal = child.GetComponent<InternalComponent>();
                        childInternal.parent = parent;
                    }
                }

                YAML::Node assets = data["Asset Registry"];
                if (assets)
                {
                    for (auto asset : assets)
                    {
                        AssetHandle handle = asset["Asset"].as<AssetHandle>();
                        std::string path = asset["Path"].as<std::string>();
                        AssetType type = Utils::StringToAssetType(asset["Type"].as<std::string>());

                        AssetManager::Import(path.c_str(), type, handle);

                        YAML::Node textureFilterNode = asset["Texture Filter"];
                        if (textureFilterNode)
                        {
                            Texture* texture = AssetManager::GetAsset<Texture>(handle);
                            texture->filter = Utils::StringToTextureFilter(textureFilterNode.as<std::string>());
                            Textures::Invalidate(*texture);
                        }
                    }
                }
            }

            void DeserializeRuntime(const char* path) { ASSERT(false, "SceneSerializer::DeserializeRuntime - Not implemented yet!"); }

            void SerializeEntity(YAML::Emitter& out, Entity& entity)
            {
                auto& internal = entity.GetComponent<InternalComponent>();
                auto& transform = entity.GetComponent<TransformComponent>();

                out << YAML::BeginMap;

                out << YAML::Key << "Entity" << YAML::Value << internal.id;

                out << YAML::Key << "Internal Component" << YAML::BeginMap;
                out << YAML::Key << "Tag" << YAML::Value << internal.tag;
                out << YAML::Key << "Is Active?" << YAML::Value << internal.isActive;
                out << YAML::Key << "Parent" << YAML::Value << ((internal.parent) ? internal.parent.GetComponent<InternalComponent>().id : 0);
                out << YAML::EndMap;

                out << YAML::Key << "Transform Component" << YAML::BeginMap;
                out << YAML::Key << "Position" << YAML::Value << transform.position;
                out << YAML::Key << "Rotation" << YAML::Value << transform.rotation;
                out << YAML::Key << "Scale" << YAML::Value << transform.scale;
                out << YAML::EndMap;

                if (entity.HasComponent<SpriteRendererComponent>())
                {
                    auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                    out << YAML::Key << "Sprite Renderer Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Texture Asset Handle" << YAML::Value << spriteRenderer.sprite;
                    out << YAML::Key << "Sorting Layer" << YAML::Value << spriteRenderer.sortingLayer;
                    out << YAML::Key << "Origin" << YAML::Value << spriteRenderer.origin;
                    out << YAML::Key << "Origin Mode" << YAML::Value << Utils::OriginModeToString(spriteRenderer.originMode);
                    out << YAML::Key << "Crop" << YAML::Value << spriteRenderer.crop;
                    out << YAML::Key << "Tint" << YAML::Value << spriteRenderer.tint;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<CircleRendererComponent>())
                {
                    auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();
                    out << YAML::Key << "Circle Renderer Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Radius" << YAML::Value << circleRenderer.radius;
                    out << YAML::Key << "Thickness" << YAML::Value << circleRenderer.thickness;
                    out << YAML::Key << "Fade" << YAML::Value << circleRenderer.fade;
                    out << YAML::Key << "Color" << YAML::Value << circleRenderer.color;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<Animator2DComponent>())
                {
                    auto& animator2D = entity.GetComponent<Animator2DComponent>();
                    out << YAML::Key << "Animator2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Animation" << YAML::Value << animator2D.animation;
                    /*
                                out << YAML::Key << "Active Slot" << YAML::Value << animator2D.controller.activeSlot;
                                out << YAML::Key << "Animation Count" << YAML::Value << animator2D.controller.animations.size();

                                for (u32 i = 0; i < animator2D.controller.animations.size(); i++)
                                {
                                    AssetHandle handle = animator2D.controller.animations[i];
                                    out << YAML::Key << "Animation " + std::to_string(i) << YAML::Value << handle;
                                }*/

                    out << YAML::EndMap;
                }

                if (entity.HasComponent<Camera2DComponent>())
                {
                    auto& cameraComponent = entity.GetComponent<Camera2DComponent>();
                    out << YAML::Key << "Camera2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Is Primary?" << YAML::Value << cameraComponent.isPrimary;
                    out << YAML::Key << "Target" << YAML::Value << cameraComponent.camera.target;
                    out << YAML::Key << "Offset" << YAML::Value << cameraComponent.camera.offset;
                    out << YAML::Key << "Rotation" << YAML::Value << cameraComponent.camera.rotation;
                    out << YAML::Key << "Zoom" << YAML::Value << cameraComponent.camera.zoom;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<Rigidbody2DComponent>())
                {
                    auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
                    out << YAML::Key << "Rigidbody2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Type" << YAML::Value << Utils::BodyTypeToString(rb2D.type);
                    out << YAML::Key << "Fixed Rotation?" << YAML::Value << rb2D.hasFixedRotation;
                    out << YAML::Key << "Gravity Scale" << YAML::Value << rb2D.gravityScale;
                    out << YAML::Key << "Linear Damping" << YAML::Value << rb2D.linearDamping;
                    out << YAML::Key << "Angular Damping" << YAML::Value << rb2D.angularDamping;
                    out << YAML::Key << "Linear Velocity" << YAML::Value << rb2D.linearVelocity;
                    out << YAML::Key << "Angular Velocity" << YAML::Value << rb2D.angularVelocity;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<BoxCollider2DComponent>())
                {
                    auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                    out << YAML::Key << "Box Collider2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Offset" << YAML::Value << bc2d.offset;
                    out << YAML::Key << "Size" << YAML::Value << bc2d.size;
                    out << YAML::Key << "Is Trigger?" << YAML::Value << bc2d.isTrigger;
                    out << YAML::Key << "Density" << YAML::Value << bc2d.density;
                    out << YAML::Key << "Friction" << YAML::Value << bc2d.friction;
                    out << YAML::Key << "Restitution" << YAML::Value << bc2d.restitution;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<NativeScriptComponent>())
                {
                    auto& nsc = entity.GetComponent<NativeScriptComponent>();
                    out << YAML::Key << "Native Script Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Script Name" << YAML::Value << nsc.scriptName;
                    out << YAML::EndMap;
                }

                out << YAML::EndMap;
            }

            void DeserializeEntity(Entity& entity, YAML::Node& node)
            {
                auto internalNode = node["Internal Component"];
                auto& internal = entity.GetComponent<InternalComponent>();
                internal.tag = internalNode["Tag"].as<std::string>();
                internal.isActive = internalNode["Is Active?"].as<bool>();

                auto transformNode = node["Transform Component"];
                auto& transform = entity.GetComponent<TransformComponent>();
                transform.position = transformNode["Position"].as<glm::vec3>();
                transform.rotation = transformNode["Rotation"].as<glm::vec3>();
                transform.scale = transformNode["Scale"].as<glm::vec3>();

                auto circleRendererNode = node["Circle Renderer Component"];
                if (circleRendererNode)
                {
                    auto& circleRenderer = entity.AddComponent<CircleRendererComponent>();
                    circleRenderer.radius = circleRendererNode["Radius"].as<float>();
                    circleRenderer.thickness = circleRendererNode["Thickness"].as<float>();
                    circleRenderer.fade = circleRendererNode["Fade"].as<float>();
                    circleRenderer.color = circleRendererNode["Color"].as<glm::vec3>();
                }

                auto spriteRendererNode = node["Sprite Renderer Component"];
                if (spriteRendererNode)
                {
                    auto& spriteRenderer = entity.AddComponent<SpriteRendererComponent>();
                    spriteRenderer.sprite = spriteRendererNode["Texture Asset Handle"].as<AssetHandle>();
                    spriteRenderer.sortingLayer = spriteRendererNode["Sorting Layer"].as<s32>();
                    spriteRenderer.origin = spriteRendererNode["Origin"].as<glm::vec2>();
                    spriteRenderer.originMode = Utils::StringToOriginMode(spriteRendererNode["Origin Mode"].as<std::string>());
                    spriteRenderer.crop = spriteRendererNode["Crop"].as<Rectangle>();
                    spriteRenderer.tint = spriteRendererNode["Tint"].as<glm::vec4>();
                }

                auto animator2DNode = node["Animator2D Component"];
                if (animator2DNode)
                {
                    auto& animator2D = entity.AddComponent<Animator2DComponent>();
                    animator2D.animation = animator2DNode["Animation"].as<AssetHandle>();
                    // animator2D.controller.activeSlot = animator2DNode["Active Slot"].as<s32>();

                    /*
                                u32 animationCount = animator2DNode["Animation Count"].as<u32>();
                                animator2D.controller.animations.resize(animationCount);
                                for (u32 i = 0; i < animationCount; i++)
                                    animator2D.controller.animations[i] = animator2DNode["Animation " + std::to_string(i)].as<AssetHandle>();*/
                }

                auto camera2DNode = node["Camera2D Component"];
                if (camera2DNode)
                {
                    auto& cameraComponent = entity.AddComponent<Camera2DComponent>();
                    cameraComponent.isPrimary = camera2DNode["Is Primary?"].as<bool>();
                    cameraComponent.camera.target = camera2DNode["Target"].as<glm::vec2>();
                    cameraComponent.camera.offset = camera2DNode["Offset"].as<glm::vec2>();
                    cameraComponent.camera.rotation = camera2DNode["Rotation"].as<float>();
                    cameraComponent.camera.zoom = camera2DNode["Zoom"].as<float>();
                }

                auto rb2DNode = node["Rigidbody2D Component"];
                if (rb2DNode)
                {
                    auto& rb2D = entity.AddComponent<Rigidbody2DComponent>();
                    rb2D.type = Utils::StringToBodyType(rb2DNode["Type"].as<std::string>());
                    rb2D.hasFixedRotation = rb2DNode["Fixed Rotation?"].as<bool>();
                    rb2D.gravityScale = rb2DNode["Gravity Scale"].as<float>();
                    rb2D.linearDamping = rb2DNode["Linear Damping"].as<float>();
                    rb2D.angularDamping = rb2DNode["Angular Damping"].as<float>();
                    rb2D.linearVelocity = rb2DNode["Linear Velocity"].as<glm::vec2>();
                    rb2D.angularVelocity = rb2DNode["Angular Velocity"].as<float>();
                }

                auto bc2DNode = node["Box Collider2D Component"];
                if (bc2DNode)
                {
                    auto& bc2D = entity.AddComponent<BoxCollider2DComponent>();
                    bc2D.offset = bc2DNode["Offset"].as<glm::vec2>();
                    bc2D.size = bc2DNode["Size"].as<glm::vec2>();
                    bc2D.isTrigger = bc2DNode["Is Trigger?"].as<bool>();
                    bc2D.density = bc2DNode["Density"].as<float>();
                    bc2D.friction = bc2DNode["Friction"].as<float>();
                    bc2D.restitution = bc2DNode["Restitution"].as<float>();
                }

                auto nsNode = node["Native Script Component"];
                if (nsNode)
                {
                    auto& nsc = entity.AddComponent<NativeScriptComponent>();
                    nsc.scriptName = nsNode["Script Name"].as<std::string>();
                }
            }
        }
    }
}
