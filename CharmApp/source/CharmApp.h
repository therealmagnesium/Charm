#pragma once
#include <Charm.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    struct CharmState
    {
        bool showDebugUI = false;
        Camera2D camera;
        glm::vec2 playerPosition;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();
}
