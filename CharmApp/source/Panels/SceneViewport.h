#pragma once
#include <Graphics/Framebuffer.h>
#include <glm/glm.hpp>

using namespace Charm::Graphics;

namespace CharmApp
{
    struct SceneViewportState
    {
        glm::vec2 position;
        glm::vec2 size;
        bool isHovered = false;
        bool isFocused = false;
    };

    namespace SceneViewportPanel
    {
        void Display(Texture& renderTarget);

        bool IsHovered();
        bool IsFocused();
        const glm::vec2& GetPosition();
        const glm::vec2& GetSize();
    }
}
