#include "Core/Utils.h"
#include "Core/Application.h"
#include "Core/Asset.h"

#include "ECS/PhysicsWorld.h"

#include "Graphics/Window.h"

#include <string>
#include <filesystem>
#include <cmath>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <box2d/types.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;
using namespace Charm::Projects;

namespace Charm
{
    namespace Utils
    {
        std::filesystem::path GetHomeDirectory()
        {
#ifdef CH_PLATFORM_LINUX
            const char* environmentVariable = "HOME";
#endif

#ifdef CH_PLATFORM_WINDOWS
            const char* environmentVariable = "USERPROFILE";
#endif
            std::filesystem::path homePath = std::getenv(environmentVariable);
            return homePath;
        }

        std::string ProjectTypeToString(Projects::ProjectType type) { return type == ProjectType::TwoDimensional ? "2D" : "3D"; }
        Projects::ProjectType StringToProjectType(const std::string& str) { return str == "2D" ? ProjectType::TwoDimensional : ProjectType::ThreeDimensional; }

        u32 GetDigitCount(s32 number) { return number != 0 ? (u32)floor(log10(abs((number)) + 1)) : 1; }
        const char* BoolToCString(bool value) { return (value) ? "true" : "false"; }
        std::string SpriteSheetAnimTypeToString(const SpriteSheetAnimType type) { return type == SpriteSheetAnimType::Horizontal ? "Horizontal" : "Vertical"; }
        Graphics::SpriteSheetAnimType StringToSpriteSheetAnimType(const std::string& str) { return str == "Horizontal" ? SpriteSheetAnimType::Horizontal : SpriteSheetAnimType::Vertical; }

        std::string TextureModeToString(TextureMode mode)
        {
            const char* modes[(u32)TextureMode::_TotalCount] = {"Single", "Sprite Sheet", "Tileset"};
            return modes[(u32)mode];
        }

        u32 TextureFilterToGL(TextureFilter filter)
        {
            u32 glFilters[6] = {GL_LINEAR, GL_NEAREST,
                                GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST,
                                GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST};
            const u32 index = (u32)filter;
            const u32 glFilter = glFilters[index];
            return glFilter;
        }

        std::string TextureFilterToString(TextureFilter filter)
        {
            const char* filters[(u32)TextureFilter::_TotalCount] = {"Linear", "Nearest",
                                                                    "Linear Mipmap Linear", "Linear Mipmap Nearest",
                                                                    "Nearest Mipmap Linear", "Nearest Mipmap Nearest"};
            const u32 index = (u32)filter;
            return filters[index];
        }

        TextureFilter StringToTextureFilter(const std::string& str)
        {
            std::unordered_map<std::string, TextureFilter> stringToFilter;
            stringToFilter["Linear"] = TextureFilter::Linear;
            stringToFilter["Nearest"] = TextureFilter::Nearest;
            stringToFilter["Linear Mipmap Linear"] = TextureFilter::LinearMipmapLinear;
            stringToFilter["Linear Mipmap Nearest"] = TextureFilter::LinearMipmapNearest;
            stringToFilter["Nearest Mipmap Linear"] = TextureFilter::NearestMipmapLinear;
            stringToFilter["Nearest Mipmap Nearest"] = TextureFilter::NearestMipmapNearest;

            return stringToFilter.at(str);
        }

        TextureMode StringToTextureMode(const std::string& str)
        {
            std::unordered_map<std::string, TextureMode> stringToMode;
            stringToMode["Single"] = TextureMode::Single;
            stringToMode["Sprite Sheet"] = TextureMode::SpriteSheet;
            stringToMode["Tileset"] = TextureMode::Tileset;

            return stringToMode.at(str);
        }

        bool IsDepthFormat(TextureFormat format)
        {
            bool value = false;

            switch (format)
            {
                case TextureFormat::Depth:
                    value = true;
                    break;
                case TextureFormat::DepthStencil:
                    value = true;
                    break;

                default:
                    break;
            }

            return value;
        }

        std::string GetFileName(const char* path, bool hasExtension)
        {
            std::filesystem::path p(path);
            std::string fileName = (!hasExtension) ? p.stem().string() : p.filename().string();
            return fileName;
        }

        std::string AssetTypeToString(AssetType type)
        {
            const char* types[(u32)AssetType::_TotalCount] = {
                "Invalid",
                "Animation",
                "Animation Controller",
                "Shader",
                "Texture",
                "Tile Palette",
                "Material",
                "Mesh",
            };
            return types[(u8)type];
        }

        FileDialogFilter AssetTypeToFileDialogFilter(Core::AssetType type)
        {
            FileDialogFilter filter;

            switch (type)
            {
                case AssetType::Animation:
                    filter.name = "Aniation";
                    filter.specification = "anim";
                    break;
                case AssetType::AnimationController:
                    filter.name = "Aniation Controller";
                    filter.specification = "ac";
                    break;
                case AssetType::Material:
                    filter.name = "Material";
                    filter.specification = "chmat";
                    break;
                case AssetType::Texture:
                    filter.name = "Texture";
                    filter.specification = "png";
                    break;
                default:
                    break;
            }

            return filter;
        }

        AssetType StringToAssetType(const std::string& str)
        {
            std::unordered_map<std::string, AssetType> list;
            list["Invalid"] = AssetType::Invalid;
            list["Animation"] = AssetType::Animation;
            list["Animation Controller"] = AssetType::AnimationController;
            list["Material"] = AssetType::Material;
            list["Mesh"] = AssetType::Model;
            list["Shader"] = AssetType::Shader;
            list["Texture"] = AssetType::Texture;
            list["Tile Palette"] = AssetType::TilePalette;

            return list.find(str) != list.end() ? list.at(str) : AssetType::Invalid;
        }

        AssetType ExtensionToAssetType(const std::string& extension)
        {
            std::unordered_map<std::string, AssetType> list;
            list[".png"] = AssetType::Texture;
            list[".jpg"] = AssetType::Texture;
            list[".jpeg"] = AssetType::Texture;
            list[".chmat"] = AssetType::Material;
            list[".glb"] = AssetType::Model;
            list[".gltf"] = AssetType::Model;
            list[".fbx"] = AssetType::Model;
            list[".obj"] = AssetType::Model;
            list[".glsl"] = AssetType::Shader;
            list[".anim"] = AssetType::Animation;
            list[".ac"] = AssetType::AnimationController;
            list[".chtp"] = AssetType::TilePalette;

            return list.find(extension) != list.end() ? list.at(extension) : AssetType::Invalid;
        }

        PhysicsBodyType StringToBodyType(const std::string& str)
        {
            PhysicsBodyType type = PhysicsBodyType::Static;

            if (str == "Dynamic")
                type = PhysicsBodyType::Dynamic;

            if (str == "Kinematic")
                type = PhysicsBodyType::Kinematic;

            return type;
        }

        std::string BodyTypeToString(PhysicsBodyType type)
        {
            switch (type)
            {
                case PhysicsBodyType::Static:
                    return "Static";

                case PhysicsBodyType::Dynamic:
                    return "Dynamic";

                case PhysicsBodyType::Kinematic:
                    return "Kinematic";

                default:
                    return "Static";
            }
        }

        u32 BodyTypeToB2BodyType(PhysicsBodyType type)
        {
            switch (type)
            {
                case PhysicsBodyType::Static:
                    return b2_staticBody;

                case PhysicsBodyType::Dynamic:
                    return b2_dynamicBody;

                case PhysicsBodyType::Kinematic:
                    return b2_kinematicBody;

                default:
                    return b2_staticBody;
            }
        }

        glm::vec2 ScreenToVirtual(const glm::vec2& screenPosition)
        {
            const ApplicationConfig& config = Application::GetConfig();
            const u32 windowWidth = Window::GetWidth();
            const u32 windowHeight = Window::GetHeight();

            glm::vec2 scale;
            scale.x = (float)config.virtualWidth / (float)windowWidth;
            scale.y = (float)config.virtualHeight / (float)windowHeight;

            glm::vec2 virtualPosition;
            virtualPosition.x = screenPosition.x * scale.x;
            virtualPosition.y = screenPosition.y * scale.y;

            return virtualPosition;
        }

        glm::vec2 ScreenToViewport(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize)
        {
            const ApplicationConfig& config = Application::GetConfig();

            glm::vec2 scale;
            scale.x = viewportSize.x / (float)config.virtualWidth;
            scale.y = viewportSize.y / (float)config.virtualHeight;

            glm::vec2 position;
            position.x = (screenPosition.x - viewportPosition.x);
            position.y = (screenPosition.y - viewportPosition.y);

            position.x = glm::clamp(position.x, 0.f, viewportSize.x);
            position.y = glm::clamp(position.y, 0.f, viewportSize.y);

            position /= scale;

            return position;
        }

        glm::vec2 ScreenToViewportGL(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize)
        {
            const ApplicationConfig& config = Application::GetConfig();

            glm::vec2 scale;
            scale.x = viewportSize.x / (float)config.virtualWidth;
            scale.y = viewportSize.y / (float)config.virtualHeight;

            glm::vec2 position;
            position.x = (screenPosition.x - viewportPosition.x);
            position.y = (viewportSize.y - screenPosition.y + viewportPosition.y);

            position.x = glm::clamp(position.x, 0.f, viewportSize.x);
            position.y = glm::clamp(position.y, 0.f, viewportSize.y);

            position /= scale;

            return position;
        }

        glm::mat4 GetTransformMatrix2D(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec2& origin)
        {
            glm::mat4 transform = glm::mat4(1.f);
            transform = glm::translate(transform, position);
            transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.f, 0.f, 1.f));
            transform = glm::translate(transform, glm::vec3(-origin, 0.f));
            transform = glm::scale(transform, glm::vec3(size, 1.f));

            return transform;
        }

        glm::mat4 GetTransformMatrix3D(const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale)
        {
            glm::mat4 transform = glm::mat4(1.f);
            transform = glm::translate(transform, position);
            transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0.f, 0.f, 1.f));
            transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0.f, 1.f, 0.f));
            transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1.f, 0.f, 0.f));
            transform = glm::scale(transform, scale);
            return transform;
        }

        glm::vec2 OriginModeToVec2(OriginMode mode, const glm::vec2& position, const glm::vec2& size)
        {
            glm::vec2 origin;
            origin.x = size.x / 2.f;
            origin.y = size.y / 2.f;

            switch (mode)
            {
                case OriginMode::Left:
                    origin.x = 0.f;
                    origin.y = size.y / 2.f;
                    break;

                case OriginMode::Right:
                    origin.x = size.x;
                    origin.y = size.y / 2.f;
                    break;

                case OriginMode::BottomLeft:
                    origin.x = 0.f;
                    origin.y = 0.f;
                    break;

                case OriginMode::BottomMiddle:
                    origin.x = size.x / 2.f;
                    origin.y = 0.f;
                    break;

                case OriginMode::BottomRight:
                    origin.x = size.x;
                    origin.y = 0.f;
                    break;

                case OriginMode::TopLeft:
                    origin.x = 0.f;
                    origin.y = size.y;
                    break;

                case OriginMode::TopMiddle:
                    origin.x = size.x / 2.f;
                    origin.y = size.y;
                    break;

                case OriginMode::TopRight:
                    origin.x = size.x;
                    origin.y = size.y;
                    break;

                default:
                    break;
            }

            return origin;
        }

        OriginMode StringToOriginMode(const std::string& str)
        {
            const char* modes[9] = {"Center", "Left", "Right", "Bottom Left", "Bottom Middle", "Bottom Right", "Top Left", "Top Middle", "Top Right"};
            std::unordered_map<std::string, OriginMode> originModes;
            for (u8 i = 0; i < LEN(modes); i++)
                originModes[modes[i]] = (OriginMode)i;

            return originModes[str];
        }

        std::string OriginModeToString(OriginMode mode)
        {
            const char* modes[9] = {"Center", "Left", "Right", "Bottom Left", "Bottom Middle", "Bottom Right", "Top Left", "Top Middle", "Top Right"};
            return modes[(u8)mode];
        }

        ECS::PhysicsBodyID B2BodyToPhysicsBody(b2BodyId& body) { return *(PhysicsBodyID*)&body; }
        ECS::PhysicsShapeID B2ShapeToPhysicsShape(b2ShapeId& shape) { return *(PhysicsShapeID*)&shape; }
        ECS::PhysicsWorldID B2WorldToPhysicsWorldID(b2WorldId& world) { return *(PhysicsWorldID*)&world; }
        ECS::PhysicsWorld B2WorldDefToPhysicsWorld(b2WorldDef& worldDef) { return *(PhysicsWorld*)&worldDef; }
    }
}
