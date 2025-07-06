#pragma once
#include <glm/glm.hpp>

namespace CharmApp
{
    struct DebugStatsState
    {
        glm::vec2 virtualMouse;
        glm::vec2 viewportMouse;
        glm::vec2 glViewportMouse;
    };

    namespace DebugStatsPanel
    {
        void Display();
    }
}
