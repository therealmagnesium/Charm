#pragma once
#include <Charm.h>

using namespace Charm::Graphics;

namespace Charm
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
        void Display(Framebuffer& framebuffer);

        bool IsHovered();
        bool IsFocused();
        const glm::vec2& GetPosition();
        const glm::vec2& GetSize();
    }
}
