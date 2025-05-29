#include "Graphics/Window.h"
#include "Core/Application.h"
#include "Core/Log.h"

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
                SDL_GL_SetSwapInterval(1);

                state.handle = SDL_CreateWindow(title, width, height, flags);
                ASSERT(state.handle != NULL, "Failed to initialize the window!");

                state.context = SDL_GL_CreateContext(state.handle);
                ASSERT(state.context != NULL, "Failed to initialize the window's OpenGL context!");

                ASSERT(SDL_GL_MakeCurrent(state.handle, state.context) != false,
                       "Failed to set the window's OpenGL context");

                INFO("Window \"%s\" was successfully created with an OpenGL context", state.title.c_str());
            }

            void Shutdown()
            {
                INFO("Window \"%s\" is shutting down...", state.title.c_str());
                SDL_DestroyWindow(state.handle);
                SDL_GL_DestroyContext(state.context);
            }

            void HandleEvents()
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
                {
                    switch (event.type)
                    {
                        case SDL_EVENT_QUIT:
                            Application::Quit();
                            break;
                    }
                }
            }

            void Display()
            {
                SDL_GL_SwapWindow(state.handle);
            }
        }
    }
}
