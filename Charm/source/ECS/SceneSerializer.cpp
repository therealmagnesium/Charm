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
                if (context == NULL)
                {
                    ERROR("SceneSerializer::Serialize - The context has not been set!");
                    return;
                }

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
                    out << YAML::EndMap;
                }
                out << YAML::EndSeq;

                out << YAML::EndMap;

                std::ofstream fout(path);
                fout << out.c_str();
                fout.close();
            }

            void SerializeRuntime(const char* path) { ASSERT(false, "SceneSerializer::SerializeRuntime - Not implemented yet!"); }

            void Deserialize(const char* path)
            {
                if (context == NULL)
                {
                    ERROR("SceneSerializer::Deserialize - The context has not been set!");
                    return;
                }

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

                std::string sceneName = data["Scene"].as<std::string>();

                YAML::Node entities = data["Entities"];
                if (entities)
                {
                    for (auto entity : entities)
                    {
                        UUID uuid = entity["Entity"].as<UUID>();
                        Entity deserializedEntity = Scenes::CreateEntity(*context, uuid);
                        DeserializeEntity(deserializedEntity, entity);
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
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<BoxCollider2DComponent>())
                {
                    auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                    out << YAML::Key << "Box Collider2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Offset" << YAML::Value << bc2d.offset;
                    out << YAML::Key << "Size" << YAML::Value << bc2d.size;
                    out << YAML::Key << "Density" << YAML::Value << bc2d.density;
                    out << YAML::Key << "Friction" << YAML::Value << bc2d.friction;
                    out << YAML::Key << "Restitution" << YAML::Value << bc2d.restitution;
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
                    spriteRenderer.origin = spriteRendererNode["Origin"].as<glm::vec2>();
                    spriteRenderer.originMode = Utils::StringToOriginMode(spriteRendererNode["Origin Mode"].as<std::string>());
                    spriteRenderer.crop = spriteRendererNode["Crop"].as<Rectangle>();
                    spriteRenderer.tint = spriteRendererNode["Tint"].as<glm::vec3>();
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
                }

                auto bc2DNode = node["Box Collider2D Component"];
                if (bc2DNode)
                {
                    auto& bc2D = entity.AddComponent<BoxCollider2DComponent>();
                    bc2D.offset = bc2DNode["Offset"].as<glm::vec2>();
                    bc2D.size = bc2DNode["Size"].as<glm::vec2>();
                    bc2D.density = bc2DNode["Density"].as<float>();
                    bc2D.friction = bc2DNode["Friction"].as<float>();
                    bc2D.restitution = bc2DNode["Restitution"].as<float>();
                }
            }
        }
    }
}
