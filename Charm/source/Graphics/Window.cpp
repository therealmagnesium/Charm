#include "Graphics/Window.h"
#include "Graphics/RenderAPI.h"

#include "Core/Application.h"
#include "Core/Input.h"
#include "Core/Log.h"

#include "UI/UI.h"

#include <SDL3/SDL.h>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        static WindowState state;
        bool isInitialized = false;

        namespace Window
        {
            void Initialize(u32 width, u32 height, const char* title)
            {
                if (isInitialized)
                {
                    WARN("Cannot initialize the window more than once");
                    return;
                }

                const u32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

                state.width = width;
                state.height = height;
                state.title = title;

                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetSwapInterval(0);

                state.handle = SDL_CreateWindow(state.title.c_str(), state.width, state.height, flags);
                ASSERT(state.handle != NULL, "Failed to initialize the window!");

                state.context = SDL_GL_CreateContext((SDL_Window*)state.handle);
                ASSERT(state.context != NULL, "Failed to initialize the window's OpenGL context!");

                const bool isContextValid = SDL_GL_MakeCurrent((SDL_Window*)state.handle, (SDL_GLContext)state.context);
                ASSERT(isContextValid != false, "Failed to set the window's OpenGL context");

                INFO("Window \"%s\" was successfully created with an OpenGL context", state.title.c_str());
            }

            void Shutdown()
            {
                INFO("Window \"%s\" is shutting down...", state.title.c_str());

                SDL_DestroyWindow((SDL_Window*)state.handle);
                SDL_GL_DestroyContext((SDL_GLContext)state.context);

                isInitialized = false;
            }

            void HandleEvents()
            {
                Input::Reset();

                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    UI::HandleEvents(&event);

                    switch (event.type)
                    {
                        case SDL_EVENT_QUIT:
                            Application::Quit();
                            break;

                        case SDL_EVENT_KEY_DOWN:
                            _Input->keyboard.keysPressed[event.key.scancode] = !_Input->keyboard.keysHeld[event.key.scancode];
                            _Input->keyboard.keysHeld[event.key.scancode] = true;
                            _Input->keyboard.keysReleased[event.key.scancode] = false;
                            break;

                        case SDL_EVENT_KEY_UP:
                            _Input->keyboard.keysPressed[event.key.scancode] = false;
                            _Input->keyboard.keysHeld[event.key.scancode] = false;
                            _Input->keyboard.keysReleased[event.key.scancode] = true;
                            break;

                        case SDL_EVENT_MOUSE_MOTION:
                            _Input->mouse.position.x = event.motion.x;
                            _Input->mouse.position.y = event.motion.y;
                            _Input->mouse.relative.x = event.motion.xrel;
                            _Input->mouse.relative.y = event.motion.yrel;
                            break;

                        case SDL_EVENT_MOUSE_BUTTON_DOWN:
                            _Input->mouse.buttonsClicked[event.button.button] = !_Input->mouse.buttonsHeld[event.button.button];
                            _Input->mouse.buttonsHeld[event.button.button] = true;
                            _Input->mouse.buttonsReleased[event.button.button] = false;
                            break;

                        case SDL_EVENT_MOUSE_BUTTON_UP:
                            _Input->mouse.buttonsClicked[event.button.button] = false;
                            _Input->mouse.buttonsHeld[event.button.button] = false;
                            _Input->mouse.buttonsReleased[event.button.button] = true;
                            break;

                        case SDL_EVENT_MOUSE_WHEEL:
                            _Input->mouse.scroll.x = event.wheel.x;
                            _Input->mouse.scroll.y = event.wheel.y;
                            break;

                        case SDL_EVENT_WINDOW_RESIZED:
                            state.width = event.window.data1;
                            state.height = event.window.data2;
                            RenderAPI::SetViewport(0, 0, state.width, state.height);
                            break;

                        case SDL_EVENT_WINDOW_MAXIMIZED:
                            state.isMaximized = true;
                            break;

                        case SDL_EVENT_WINDOW_MINIMIZED:
                            state.isMinimized = true;
                            break;

                        case SDL_EVENT_WINDOW_RESTORED:
                            state.isMinimized = false;
                            state.isMaximized = false;
                            break;

                        default:
                            break;
                    }
                }

                Application::SetViewportPosition(glm::vec2(0.f));
                Application::SetViewportSize(glm::vec2(state.width, state.height));
            }

            void Display()
            {
                SDL_GL_SwapWindow((SDL_Window*)state.handle);
            }

            void* GetHandle() { return state.handle; }
            void* GetContext() { return state.context; }
            u32 GetWidth() { return state.width; }
            u32 GetHeight() { return state.height; }
            bool IsMaximized() { return state.isMaximized; }
            bool IsMinimized() { return state.isMinimized; }
        }
    }
}
