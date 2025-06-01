#pragma once
#include <Charm.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    struct CharmState
    {
        bool showDebugUI = false;
        Camera2D camera;
        u32 vertexArray;
        u32 vertexBuffer;
        u32 indexBuffer;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();
}
