#include "ECS/SceneSerializer.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

#include "Core/Log.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>

namespace YAML
{
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
}

namespace Charm
{
    namespace ECS
    {
        static Scene* context = NULL;

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

        namespace SceneSerializer
        {
            void SerializeEntity(YAML::Emitter& out, Entity& entity);

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
                out << YAML::EndMap;

                std::ofstream fout(path);
                fout << out.c_str();
                fout.close();
            }

            void SerializeRuntime(const char* path) { ASSERT(false, "SceneSerializer::SerializeRuntime - Not implemented yet!"); }

            void Deserialize(const char* path)
            {
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
                        Entity deserializedEntity = Scenes::CreateEntity(*context);

                        auto internalNode = entity["Internal Component"];
                        auto& internal = deserializedEntity.GetComponent<InternalComponent>();
                        internal.tag = internalNode["Tag"].as<std::string>();
                        internal.isActive = internalNode["Is Active?"].as<bool>();
                        internal.id = uuid;

                        auto transformNode = entity["Transform Component"];
                        auto& transform = deserializedEntity.GetComponent<TransformComponent>();
                        transform.position = transformNode["Position"].as<glm::vec3>();
                        transform.rotation = transformNode["Rotation"].as<glm::vec3>();
                        transform.scale = transformNode["Scale"].as<glm::vec3>();

                        auto circleRendererNode = entity["Circle Renderer Component"];
                        if (circleRendererNode)
                        {
                            auto& circleRenderer = deserializedEntity.AddComponent<CircleRendererComponent>();
                            circleRenderer.radius = circleRendererNode["Radius"].as<float>();
                            circleRenderer.thickness = circleRendererNode["Thickness"].as<float>();
                            circleRenderer.fade = circleRendererNode["Fade"].as<float>();
                            circleRenderer.color = circleRendererNode["Color"].as<glm::vec3>();
                        }

                        auto spriteRendererNode = entity["Sprite Renderer Component"];
                        if (spriteRendererNode)
                        {
                            auto& spriteRenderer = deserializedEntity.AddComponent<SpriteRendererComponent>();
                            spriteRenderer.sprite = spriteRendererNode["Texture Asset Handle"].as<float>();
                            spriteRenderer.tint = spriteRendererNode["Tint"].as<glm::vec3>();
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

                out << YAML::EndMap;
            }
        }
    }
}
