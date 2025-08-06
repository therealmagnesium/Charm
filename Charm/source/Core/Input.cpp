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
                state.mouse.relative = glm::vec2(0.f);
                state.mouse.scroll = glm::vec2(0.f);

                for (u32 i = 0; i < KEY_COUNT; i++)
                    state.keyboard.keysPressed[i] = false;

                for (u32 i = 0; i < MOUSE_BUTTON_COUNT; i++)
                    state.mouse.buttonsClicked[i] = false;
            }

            void Capture(bool shouldCapture) { state.shouldCapture = shouldCapture; }

            bool IsMouseDown(MouseButton button) { return (state.shouldCapture) ? state.mouse.buttonsHeld[button] : false; }
            bool IsMouseClicked(MouseButton button) { return (state.shouldCapture) ? state.mouse.buttonsClicked[button] : false; }
            glm::vec2 GetMousePosition() { return (state.shouldCapture) ? state.mouse.position : glm::vec2(0.f); }
            glm::vec2 GetMouseRelative() { return (state.shouldCapture) ? state.mouse.relative : glm::vec2(0.f); }
            glm::vec2 GetMouseScroll() { return (state.shouldCapture) ? state.mouse.scroll : glm::vec2(0.f); }

            bool IsKeyDown(u32 scancode) { return (state.shouldCapture) ? state.keyboard.keysHeld[scancode] : false; }
            bool IsKeyPressed(u32 scancode) { return (state.shouldCapture) ? state.keyboard.keysPressed[scancode] : false; }

            float GetAxis(InputAxis axis)
            {
                float value = 0.f;
                switch (axis)
                {
                    case InputAxis::Horizontal:
                        value = IsKeyDown(KEY_RIGHT) - IsKeyDown(KEY_LEFT);
                        break;

                    case InputAxis::Vertical:
                        value = IsKeyDown(KEY_UP) - IsKeyDown(KEY_DOWN);
                        break;
                }

                return value;
            }

            float GetAxisAlt(InputAxis axis)
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
