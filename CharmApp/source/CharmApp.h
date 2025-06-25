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
        AssetHandle textures[2];
        Scene scene;
        bool isEditorMode = true;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();
}
