#pragma once
#include "Core/Base.h"
#include <string>

namespace Charm
{
    namespace Graphics
    {
        struct WindowState
        {
            u32 width = 1280;
            u32 height = 720;
            std::string title = "Untitled";
            void* handle = NULL;
            void* context = NULL;
        };

        namespace Window
        {
            void Initialize(u32 width, u32 height, const char* title);
            void Shutdown();
            void HandleEvents();
            void Display();

            void* GetHandle();
            void* GetContext();
        }
    }
}
