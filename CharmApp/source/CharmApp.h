#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

namespace CharmApp
{
    struct CharmState
    {
        bool showDebugUI = false;

        Camera2D camera;
        AssetHandle textures[2];
        u32 activeTextureSlot = 0;

        Scene scene;
        Entity entity;

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
