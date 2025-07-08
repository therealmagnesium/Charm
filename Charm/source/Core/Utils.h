#pragma once
#include "Graphics/Shapes.h"
#include "Graphics/Texture.h"

#include <string>
#include <glm/glm.hpp>

namespace Charm
{
    namespace Utils
    {

        const char* BoolToCString(bool value);
        bool IsDepthFormat(Graphics::TextureFormat format);
        std::string GetFileName(const char* path, bool hasExtension = false);
        std::string AssetTypeToString(Core::AssetType type);
        Core::AssetType StringToAssetType(const std::string& str);
        Graphics::BodyType StringToBodyType(const std::string& str);
        std::string BodyTypeToString(Graphics::BodyType type);
        u32 BodyTypeToB2BodyType(Graphics::BodyType type);
        glm::vec2 ScreenToVirtual(const glm::vec2& screenPosition);
        glm::vec2 ScreenToViewport(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize);
        glm::vec2 ScreenToViewportGL(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize);
        glm::mat4 GetTransfomMatrix2D(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec2& origin);
        glm::vec2 OriginModeToVec2(Graphics::OriginMode mode, const glm::vec2& position, const glm::vec2& size);
        std::string OriginModeToString(Graphics::OriginMode mode);
        Graphics::OriginMode StringToOriginMode(const std::string& str);
    }
}
