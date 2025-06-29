#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

namespace CharmApp
{
    struct CharmState
    {
        Framebuffer framebuffer;
        Scene scene;
        Texture iconPlay;
        Texture iconStop;
        s32 pixelData = -1;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();
}
