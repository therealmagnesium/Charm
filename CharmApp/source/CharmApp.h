#pragma once
#include <Charm.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    struct CharmState
    {
        bool showDebugUI = false;

        Camera2D camera;
        Texture texture;

        float tileSize = 64.f;
        float tileSpacing = 4.f;
        float tileOffset = 0.f;

        glm::vec2 playerPosition;
        glm::vec2 playerDirection;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();
}
