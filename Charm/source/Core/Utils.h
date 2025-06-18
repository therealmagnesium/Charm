#pragma once
#include "Core/Asset.h"

#include <string>
#include <glm/glm.hpp>

using namespace Charm::Core;

namespace Charm
{
    namespace Utils
    {
        std::string GetFileName(const char* path, bool hasExtension = false);
        std::string AssetTypeToString(AssetType type);
        glm::vec2 ScreenToVirtual(const glm::vec2& screenPosition);
        glm::mat4 GetTransfomMatrix2D(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin);
    }
}
