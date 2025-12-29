#pragma once
#include <glm/glm.hpp>

namespace CharmApp
{
    struct DebugStatsState
    {
        glm::vec2 virtualMouse;
        glm::vec2 viewportMouse;
        glm::vec2 glViewportMouse;
        bool shouldDisplay = true;
    };

    namespace DebugStatsPanel
    {
        void Display();
        void Toggle();
        bool ShouldDisplay();
    }
}
