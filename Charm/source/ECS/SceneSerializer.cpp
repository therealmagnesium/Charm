#include "ECS/SceneSerializer.h"
#include "ECS/Scene.h"
#include "ECS/Entity.h"
#include "ECS/Components.h"

#include "Core/Log.h"
#include "Core/Utils.h"

#include "Graphics/TilePalette.h"

#include "Projects/Project.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <sstream>

using namespace Charm::Projects;

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

            void SetContext(Scene* scene) { context = scene; }

            void Serialize(const std::filesystem::path& path)
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
                const AssetRegistry& registry = AssetManager::GetRegistry();
                std::vector<std::pair<AssetHandle, AssetMetadata>> sortedRegistry;

                const auto SortByAssetType = [&](const std::pair<AssetHandle, AssetMetadata>& a, const std::pair<AssetHandle, AssetMetadata>& b) { return a.second.type < b.second.type; };
                std::copy(registry.begin(), registry.end(), std::back_inserter(sortedRegistry));
                std::sort(sortedRegistry.begin(), sortedRegistry.end(), SortByAssetType);

                for (auto& [handle, metadata] : sortedRegistry)
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
                            out << YAML::Key << "Texture Mode" << YAML::Value << Utils::TextureModeToString(texture->mode);
                            out << YAML::Key << "Texture Min Filter" << YAML::Value << Utils::TextureFilterToString(texture->minFilter);
                            out << YAML::Key << "Texture Mag Filter" << YAML::Value << Utils::TextureFilterToString(texture->magFilter);
                            // out << YAML::Key << "Texture Row Count" << YAML::Value << texture->rowCount;
                            // out << YAML::Key << "Texture Column Count" << YAML::Value << texture->columnCount;
                            out << YAML::Key << "Texture Pixels Per Unit" << YAML::Value << texture->pixelsPerUnit;
                            break;
                        }
                        case AssetType::Animation:
                        {
                            const Project& project = ProjectManager::GetActive();
                            const std::filesystem::path assetFilesytemPath = ProjectManager::GetAssetFileSystemPath(metadata.path, project);
                            Animation* animation = (Animation*)asset;
                            Animations::Save(assetFilesytemPath.c_str(), *animation);
                            break;
                        }
                        case AssetType::AnimationController:
                        {
                            const Project& project = ProjectManager::GetActive();
                            const std::filesystem::path assetFilesytemPath = ProjectManager::GetAssetFileSystemPath(metadata.path, project);
                            AnimationController* controller = (AnimationController*)asset;
                            Animations::SaveController(assetFilesytemPath.c_str(), *controller);
                            break;
                        }
                        case AssetType::TilePalette:
                        {
                            const Project& project = ProjectManager::GetActive();
                            const std::filesystem::path assetFilesytemPath = ProjectManager::GetAssetFileSystemPath(metadata.path, project);
                            TilePalette* tilePalette = (TilePalette*)asset;
                            TilePalettes::Save(*tilePalette);
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

            void SerializeRuntime(const std::filesystem::path& path) { ASSERT(false, "SceneSerializer::SerializeRuntime - Not implemented yet!"); }

            void Deserialize(const std::filesystem::path& path)
            {
                ASSERT_ERROR(context != NULL, "SceneSerializer::Deserialize - The context has not been set!");

                std::stringstream stream;
                std::ifstream in(path);

                ASSERT_ERROR(in.is_open(), "SceneSerializer::Deserialize - Failed to load scene \"%s\"", path.c_str());

                stream << in.rdbuf();
                in.close();

                YAML::Node data = YAML::Load(stream.str());
                const YAML::Node& sceneNode = data["Scene"];
                const YAML::Node& assetsNode = data["Asset Registry"];
                const YAML::Node& entitiesNode = data["Entities"];
                ASSERT_ERROR(data && sceneNode && assetsNode && entitiesNode, "SceneSerializer::Deserialize - Invalid scene file %s!", path.c_str());

                const Project& project = ProjectManager::GetActive();
                for (auto asset : assetsNode)
                {
                    const AssetHandle handle = asset["Asset"].as<AssetHandle>();
                    const AssetType type = Utils::StringToAssetType(asset["Type"].as<std::string>());
                    const std::filesystem::path savedPath = asset["Path"].as<std::string>();

                    AssetManager::Import(savedPath, type, handle);

                    if (type == AssetType::Texture)
                    {
                        Texture* texture = AssetManager::GetAsset<Texture>(handle);
                        texture->mode = Utils::StringToTextureMode(asset["Texture Mode"].as<std::string>());
                        texture->minFilter = Utils::StringToTextureFilter(asset["Texture Min Filter"].as<std::string>());
                        texture->magFilter = Utils::StringToTextureFilter(asset["Texture Mag Filter"].as<std::string>());
                        // texture->rowCount = asset["Texture Row Count"].as<u32>();
                        // texture->columnCount = asset["Texture Column Count"].as<u32>();
                        texture->pixelsPerUnit = asset["Texture Pixels Per Unit"].as<u32>();
                        Textures::Invalidate(*texture);
                    }
                }

                std::vector<std::pair<UUID, UUID>> parentChildRelationships;
                for (auto entity : entitiesNode)
                {
                    const UUID uuid = entity["Entity"].as<UUID>();
                    Entity deserializedEntity = Scenes::CreateEntity(*context, uuid);

                    const YAML::Node& internalNode = entity["Internal Component"];
                    const UUID parentUUID = internalNode["Parent"].as<UUID>();
                    if (parentUUID != 0)
                        parentChildRelationships.emplace_back(uuid, parentUUID);

                    try
                    {
                        DeserializeEntity(deserializedEntity, entity);
                    }
                    catch (const YAML::BadConversion& e)
                    {
                        ERROR("Failed to deserialize entity %lX!", uuid);
                    }
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

            void DeserializeRuntime(const std::filesystem::path& path) { ASSERT(false, "SceneSerializer::DeserializeRuntime - Not implemented yet!"); }

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
                    const auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                    out << YAML::Key << "Sprite Renderer Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Texture Asset Handle" << YAML::Value << spriteRenderer.sprite;
                    out << YAML::Key << "Sorting Layer" << YAML::Value << spriteRenderer.sortingLayer;
                    out << YAML::Key << "Tiling Factor" << YAML::Value << spriteRenderer.tilingFactor;
                    out << YAML::Key << "Origin" << YAML::Value << spriteRenderer.origin;
                    out << YAML::Key << "Origin Mode" << YAML::Value << Utils::OriginModeToString(spriteRenderer.originMode);
                    out << YAML::Key << "Crop" << YAML::Value << spriteRenderer.crop;
                    out << YAML::Key << "Tint" << YAML::Value << spriteRenderer.tint;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<CircleRendererComponent>())
                {
                    const auto& circleRenderer = entity.GetComponent<CircleRendererComponent>();
                    out << YAML::Key << "Circle Renderer Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Radius" << YAML::Value << circleRenderer.radius;
                    out << YAML::Key << "Thickness" << YAML::Value << circleRenderer.thickness;
                    out << YAML::Key << "Fade" << YAML::Value << circleRenderer.fade;
                    out << YAML::Key << "Color" << YAML::Value << circleRenderer.color;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<Animator2DComponent>())
                {
                    const auto& animator2D = entity.GetComponent<Animator2DComponent>();
                    out << YAML::Key << "Animator2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Controller" << YAML::Value << animator2D.controller;
                    out << YAML::Key << "Active Slot" << YAML::Value << animator2D.activeSlot;

                    out << YAML::EndMap;
                }

                if (entity.HasComponent<Camera2DComponent>())
                {
                    const auto& cameraComponent = entity.GetComponent<Camera2DComponent>();
                    out << YAML::Key << "Camera2D Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Is Primary?" << YAML::Value << cameraComponent.isPrimary;
                    out << YAML::Key << "Clear Color" << YAML::Value << cameraComponent.clearColor;
                    out << YAML::Key << "Target" << YAML::Value << cameraComponent.camera.target;
                    out << YAML::Key << "Offset" << YAML::Value << cameraComponent.camera.offset;
                    out << YAML::Key << "Rotation" << YAML::Value << cameraComponent.camera.rotation;
                    out << YAML::Key << "Zoom" << YAML::Value << cameraComponent.camera.zoom;
                    out << YAML::EndMap;
                }

                if (entity.HasComponent<Rigidbody2DComponent>())
                {
                    const auto& rb2D = entity.GetComponent<Rigidbody2DComponent>();
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
                    const auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
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
                    const auto& nsc = entity.GetComponent<NativeScriptComponent>();
                    out << YAML::Key << "Native Script Component" << YAML::Value << YAML::BeginMap;
                    out << YAML::Key << "Script Name" << YAML::Value << nsc.scriptName;
                    out << YAML::EndMap;
                }

                const Project& project = ProjectManager::GetActive();
                if (project.type == ProjectType::ThreeDimensional)
                {
                    if (entity.HasComponent<MeshRendererComponent>())
                    {
                        const auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
                        out << YAML::Key << "Mesh Renderer Component" << YAML::Value << YAML::BeginMap;
                        out << YAML::Key << "Model" << YAML::Value << meshRenderer.model;
                        out << YAML::Key << "Submesh Index" << YAML::Value << meshRenderer.submeshIndex;
                        out << YAML::EndMap;
                    }

                    if (entity.HasComponent<DirectionalLightComponent>())
                    {
                        const auto& dlc = entity.GetComponent<DirectionalLightComponent>();
                        out << YAML::Key << "Directional Light Component" << YAML::Value << YAML::BeginMap;
                        out << YAML::Key << "Direction" << YAML::Value << dlc.sun.direction;
                        out << YAML::Key << "Color" << YAML::Value << dlc.sun.color;
                        out << YAML::Key << "Intensity" << YAML::Value << dlc.sun.intensity;
                        out << YAML::EndMap;
                    }

                    if (entity.HasComponent<Camera3DComponent>())
                    {
                        const auto& cc = entity.GetComponent<Camera3DComponent>();
                        out << YAML::Key << "Camera3D Component" << YAML::Value << YAML::BeginMap;
                        out << YAML::Key << "Is Primary?" << YAML::Value << cc.isPrimary;
                        out << YAML::Key << "Clear Color" << YAML::Value << cc.clearColor;
                        out << YAML::Key << "Position" << YAML::Value << cc.camera.position;
                        out << YAML::Key << "Rotation" << YAML::Value << cc.camera.rotation;
                        out << YAML::Key << "FOV" << YAML::Value << cc.camera.fov;
                        out << YAML::Key << "Near Clip" << YAML::Value << cc.camera.nearClip;
                        out << YAML::Key << "Far Clip" << YAML::Value << cc.camera.farClip;
                        out << YAML::EndMap;
                    }
                }

                out << YAML::EndMap;
            }

            void DeserializeEntity(Entity& entity, YAML::Node& node)
            {
                const YAML::Node& internalNode = node["Internal Component"];
                auto& internal = entity.GetComponent<InternalComponent>();
                internal.tag = internalNode["Tag"].as<std::string>();
                internal.isActive = internalNode["Is Active?"].as<bool>();

                const YAML::Node& transformNode = node["Transform Component"];
                auto& transform = entity.GetComponent<TransformComponent>();
                transform.position = transformNode["Position"].as<glm::vec3>();
                transform.rotation = transformNode["Rotation"].as<glm::vec3>();
                transform.scale = transformNode["Scale"].as<glm::vec3>();

                const YAML::Node& circleRendererNode = node["Circle Renderer Component"];
                if (circleRendererNode)
                {
                    auto& circleRenderer = entity.AddComponent<CircleRendererComponent>();
                    circleRenderer.radius = circleRendererNode["Radius"].as<float>();
                    circleRenderer.thickness = circleRendererNode["Thickness"].as<float>();
                    circleRenderer.fade = circleRendererNode["Fade"].as<float>();
                    circleRenderer.color = circleRendererNode["Color"].as<glm::vec3>();
                }

                const YAML::Node& spriteRendererNode = node["Sprite Renderer Component"];
                if (spriteRendererNode)
                {
                    auto& spriteRenderer = entity.AddComponent<SpriteRendererComponent>();
                    spriteRenderer.sprite = spriteRendererNode["Texture Asset Handle"].as<AssetHandle>();
                    spriteRenderer.sortingLayer = spriteRendererNode["Sorting Layer"].as<s32>();
                    spriteRenderer.tilingFactor = spriteRendererNode["Tiling Factor"].as<glm::vec2>();
                    spriteRenderer.origin = spriteRendererNode["Origin"].as<glm::vec2>();
                    spriteRenderer.originMode = Utils::StringToOriginMode(spriteRendererNode["Origin Mode"].as<std::string>());
                    spriteRenderer.crop = spriteRendererNode["Crop"].as<Rectangle>();
                    spriteRenderer.tint = spriteRendererNode["Tint"].as<glm::vec4>();
                }

                const YAML::Node& animator2DNode = node["Animator2D Component"];
                if (animator2DNode)
                {
                    auto& animator2D = entity.AddComponent<Animator2DComponent>();
                    animator2D.controller = animator2DNode["Controller"].as<AssetHandle>();
                    animator2D.activeSlot = animator2DNode["Active Slot"].as<s32>();
                }

                const YAML::Node& camera2DNode = node["Camera2D Component"];
                if (camera2DNode)
                {
                    auto& cameraComponent = entity.AddComponent<Camera2DComponent>();
                    cameraComponent.isPrimary = camera2DNode["Is Primary?"].as<bool>();
                    cameraComponent.clearColor = camera2DNode["Clear Color"].as<glm::vec4>();
                    cameraComponent.camera.target = camera2DNode["Target"].as<glm::vec2>();
                    cameraComponent.camera.offset = camera2DNode["Offset"].as<glm::vec2>();
                    cameraComponent.camera.rotation = camera2DNode["Rotation"].as<float>();
                    cameraComponent.camera.zoom = camera2DNode["Zoom"].as<float>();
                }

                const YAML::Node& rb2DNode = node["Rigidbody2D Component"];
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

                const YAML::Node& bc2DNode = node["Box Collider2D Component"];
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

                const YAML::Node& nsNode = node["Native Script Component"];
                if (nsNode)
                {
                    auto& nsc = entity.AddComponent<NativeScriptComponent>();
                    nsc.scriptName = nsNode["Script Name"].as<std::string>();
                }

                const YAML::Node& meshRendererNode = node["Mesh Renderer Component"];
                if (meshRendererNode)
                {
                    auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
                    meshRenderer.model = meshRendererNode["Model"].as<AssetHandle>();
                    meshRenderer.submeshIndex = meshRendererNode["Submesh Index"].as<s32>();
                }

                const YAML::Node& directionalLightNode = node["Directional Light Component"];
                if (directionalLightNode)
                {
                    auto& dlc = entity.AddComponent<DirectionalLightComponent>();
                    dlc.sun.direction = directionalLightNode["Direction"].as<glm::vec3>();
                    dlc.sun.color = directionalLightNode["Color"].as<glm::vec3>();
                    dlc.sun.intensity = directionalLightNode["Intensity"].as<float>();
                }

                const YAML::Node& ccNode = node["Camera3D Component"];
                if (ccNode)
                {
                    auto& cc = entity.AddComponent<Camera3DComponent>();
                    cc.isPrimary = ccNode["Is Primary?"].as<bool>();
                    cc.clearColor = ccNode["Clear Color"].as<glm::vec4>();
                    cc.camera.position = ccNode["Position"].as<glm::vec3>();
                    cc.camera.rotation = ccNode["Rotation"].as<glm::vec3>();
                    cc.camera.fov = ccNode["FOV"].as<float>();
                    cc.camera.nearClip = ccNode["Near Clip"].as<float>();
                    cc.camera.farClip = ccNode["Far Clip"].as<float>();
                }
            }
        }
    }
}
