#pragma once
#include "Core/Base.h"
#include <string>
#include <SDL3/SDL_video.h>

namespace Charm
{
    namespace Graphics
    {
        struct WindowState
        {
            u32 width = 1280;
            u32 height = 720;
            std::string title = "Untitled";
            SDL_Window* handle = NULL;
            SDL_GLContextState* context = NULL;
        };

        namespace Window
        {
            void Initialize(u32 width, u32 height, const char* title);
            void Shutdown();
            void HandleEvents();
            void Display();
        }
    }
}
