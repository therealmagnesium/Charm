#include "CharmApp.h"

#include <Charm.h>
#include <imgui.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static CharmState state;

    void OnCreate()
    {
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);

        state.defaultShader.Load("assets/shaders/Default_vs.glsl", "assets/shaders/Default_fs.glsl");
    }

    void OnUpdate()
    {
        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();
    }

    void OnRender() {}

    void OnRenderUI() { ImGui::ShowDemoWindow(); }

    void OnShutdown() {}
}
