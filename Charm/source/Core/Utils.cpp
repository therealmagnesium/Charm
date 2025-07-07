#include "Core/Utils.h"
#include "Core/Application.h"
#include "Core/Asset.h"
#include "Graphics/Window.h"

#include <string>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <box2d/types.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace Utils
    {
        const char* BoolToCString(bool value)
        {
            const char* x = value ? "true" : "false";
            return x;
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
            const char* types[3] = {"Invalid", "Texture", "Shader"};
            return types[(u8)type];
        }

        AssetType StringToAssetType(const std::string& str)
        {
            AssetType type = AssetType::Invalid;

            if (str == "Texture")
                type = AssetType::Texture;

            if (str == "Shader")
                type = AssetType::Shader;

            return type;
        }

        BodyType StringToBodyType(const std::string& str)
        {
            BodyType type = BodyType::Static;

            if (str == "Dynamic")
                type = BodyType::Dynamic;

            if (str == "Kinematic")
                type = BodyType::Kinematic;

            return type;
        }

        std::string BodyTypeToString(BodyType type)
        {
            switch (type)
            {
                case BodyType::Static:
                    return "Static";

                case BodyType::Dynamic:
                    return "Dynamic";

                case BodyType::Kinematic:
                    return "Kinematic";

                default:
                    return "Static";
            }
        }

        u32 BodyTypeToB2BodyType(BodyType type)
        {
            switch (type)
            {
                case BodyType::Static:
                    return b2_staticBody;

                case BodyType::Dynamic:
                    return b2_dynamicBody;

                case BodyType::Kinematic:
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

        glm::mat4 GetTransfomMatrix2D(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec2& origin)
        {
            glm::mat4 transform = glm::mat4(1.f);
            transform = glm::translate(transform, position);
            transform = glm::rotate(transform, glm::radians(rotation), glm::vec3(0.f, 0.f, 1.f));
            transform = glm::translate(transform, glm::vec3(-origin, 0.f));
            transform = glm::scale(transform, glm::vec3(size, 1.f));

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
    }
}
