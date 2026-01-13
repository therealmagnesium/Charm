#pragma once
#include <Graphics/Texture.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    enum class ToolbarIcon : u8
    {
        Play = 0,
        Stop,
        Translate,
        Rotate,
        Scale,
        Count
    };

    struct ToolbarState
    {
        Texture icons[(u32)ToolbarIcon::Count];
        u32 windowFlags = 0;
        u32 manipulationType = 0;
    };

    namespace ToolbarPanel
    {
        void Init();
        void Shutdown();
        void Display();

        u32 GetManipulationType();
        const Texture& GetIcon(ToolbarIcon type);
        void SetManipulationType(u32 type);
    }
}
