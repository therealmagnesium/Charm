#include "CharmApp.h"

#include <Graphics/Renderer.h>

using namespace Charm::Graphics;

namespace CharmApp
{
    void OnCreate()
    {
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);
    }

    void OnUpdate() {}
    void OnRender() {}
    void OnRenderUI() {}
    void OnShutdown() {}
}
