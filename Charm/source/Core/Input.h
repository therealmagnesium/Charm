#pragma once
#include "Core/Base.h"
#include "Core/KeyCodes.h"
#include <glm/glm.hpp>

namespace Charm
{
    namespace Core
    {
        enum class InputAxis
        {
            Horizontal = 0,
            Vertical,
        };

        struct InputMouseState
        {
            glm::vec2 position;
            glm::vec2 relative;
            bool buttonsHeld[MOUSE_BUTTON_COUNT];
            bool buttonsClicked[MOUSE_BUTTON_COUNT];
        };

        struct InputKeyboardState
        {
            bool keysHeld[KEY_COUNT];
            bool keysPressed[KEY_COUNT];
        };

        struct InputState
        {
            InputMouseState mouse;
            InputKeyboardState keyboard;
        };

        extern InputState* _Input; // Should only be used by the window

        namespace Input
        {
            void Initialize();
            void Reset();

            bool IsMouseDown(MouseButton button);
            bool IsMouseClicked(MouseButton button);
            glm::vec2& GetMousePosition();
            glm::vec2& GetMouseRelative();

            bool IsKeyDown(u32 scancode);
            bool IsKeyPressed(u32 scancode);
            float GetInputAxis(InputAxis axis);
            float GetInputAxisAlt(InputAxis axis);
        }
    }
}
