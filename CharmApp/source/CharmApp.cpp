#include "CharmApp.h"

#include <Core/Application.h>
#include <Core/Input.h>
#include <Core/Log.h>
#include <Core/Time.h>

#include <Graphics/Renderer.h>

#include <imgui.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    void OnCreate()
    {
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);
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
