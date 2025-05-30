#include "Core/Input.h"
#include "Core/Base.h"
#include "Core/KeyCodes.h"
#include <glm/glm.hpp>

namespace Charm
{
    namespace Core
    {
        static InputState state;
        InputState* _Input = NULL;

        namespace Input
        {
            void Initialize() { _Input = &state; }

            void Reset()
            {
                for (u32 i = 0; i < KEY_COUNT; i++)
                    state.keyboard.keysPressed[i] = false;

                for (u32 i = 0; i < MOUSE_BUTTON_COUNT; i++)
                    state.mouse.buttonsClicked[i] = false;
            }

            bool IsMouseDown(MouseButton button) { return state.mouse.buttonsHeld[button]; }
            bool IsMouseClicked(MouseButton button) { return state.mouse.buttonsClicked[button]; }
            glm::vec2& GetMousePosition() { return state.mouse.position; }
            glm::vec2& GetMouseRelative() { return state.mouse.relative; }

            bool IsKeyDown(u32 scancode) { return state.keyboard.keysHeld[scancode]; }
            bool IsKeyPressed(u32 scancode) { return state.keyboard.keysPressed[scancode]; }

            float GetInputAxis(InputAxis axis)
            {
                float value = 0.f;
                switch (axis)
                {
                    case InputAxis::Horizontal:
                        value = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
                        break;

                    case InputAxis::Vertical:
                        value = IsKeyDown(KEY_DOWN) - IsKeyDown(KEY_UP);
                        break;
                }

                return value;
            }

            float GetInputAxisAlt(InputAxis axis)
            {
                float value = 0.f;
                switch (axis)
                {
                    case InputAxis::Horizontal:
                        value = IsKeyDown(KEY_D) - IsKeyDown(KEY_A);
                        break;

                    case InputAxis::Vertical:
                        value = IsKeyDown(KEY_S) - IsKeyDown(KEY_W);
                        break;
                }

                return value;
            }
        }
    }
}
