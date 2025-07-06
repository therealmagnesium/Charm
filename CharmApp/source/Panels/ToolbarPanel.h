#pragma once
#include <Graphics/Texture.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    struct ToolbarState
    {
        Texture iconPlay;
        Texture iconStop;
    };

    namespace ToolbarPanel
    {
        void Init();
        void Shutdown();
        void Display();
    }
}
