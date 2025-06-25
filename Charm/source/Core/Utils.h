#pragma once
#include "Core/Asset.h"
#include "Graphics/Texture.h"

#include <string>
#include <glm/glm.hpp>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace Charm
{
    namespace Utils
    {
        const char* BoolToCString(bool value);
        bool IsDepthFormat(TextureFormat format);
        std::string GetFileName(const char* path, bool hasExtension = false);
        std::string AssetTypeToString(AssetType type);
        glm::vec2 ScreenToVirtual(const glm::vec2& screenPosition);
        glm::vec2 ScreenToViewport(const glm::vec2& screenPosition, const glm::vec2& viewportPosition, const glm::vec2& viewportSize);
        glm::mat4 GetTransfomMatrix2D(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin);
    }
}
