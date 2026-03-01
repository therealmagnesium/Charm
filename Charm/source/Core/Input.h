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
            glm::vec2 scroll;
            bool buttonsHeld[MOUSE_BUTTON_COUNT];
            bool buttonsClicked[MOUSE_BUTTON_COUNT];
            bool buttonsReleased[MOUSE_BUTTON_COUNT];
        };

        struct InputKeyboardState
        {
            bool keysHeld[KEY_COUNT];
            bool keysPressed[KEY_COUNT];
            bool keysReleased[KEY_COUNT];
        };

        struct InputState
        {
            bool shouldCapture = true;
            InputMouseState mouse;
            InputKeyboardState keyboard;
        };

        extern InputState* _Input; // Should only be used by the window

        namespace Input
        {
            void Initialize();
            void Reset();
            void Capture(bool shouldCapture);

            bool GetCapture();

            bool IsMouseDown(MouseButton button);
            bool IsMouseClicked(MouseButton button);
            bool IsMouseReleased(MouseButton button);
            glm::vec2 GetMousePosition();
            glm::vec2 GetMouseRelative();
            glm::vec2 GetMouseScroll();

            bool IsKeyDown(KeyboardKey scancode);
            bool IsKeyPressed(KeyboardKey scancode);
            bool IsKeyReleased(KeyboardKey scancode);
            float GetAxis(InputAxis axis);
            float GetAxisAlt(InputAxis axis);
        }
    }
}
