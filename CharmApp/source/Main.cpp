#include "CharmApp.h"
#include "CharmHub.h"

#include <Core/Application.h>
#include <Core/Log.h>

using namespace Charm::Core;

int main(int argc, char** argv)
{
    ApplicationConfig config;
    config.name = "Charm Project Hub";
    config.author = "Magnus Ahlstromer V";
    config.virtualWidth = 1280;
    config.virtualHeight = 720;
    config.funcs.OnCreate = CharmHub::OnCreate;
    config.funcs.OnUpdate = CharmHub::OnUpdate;
    config.funcs.OnRender = CharmHub::OnRender;
    config.funcs.OnRenderUI = CharmHub::OnRenderUI;
    config.funcs.OnShutdown = CharmHub::OnShutdown;

    Application::Setup(config);
    Application::Run();
    Application::Shutdown();

    if (CharmHub::IsProjectSelected())
    {
        config.name = "Charm Editor";
        config.author = "Magnus Ahlstromer V";
        config.virtualWidth = 1280;
        config.virtualHeight = 720;
        config.funcs.OnCreate = CharmApp::OnCreate;
        config.funcs.OnUpdate = CharmApp::OnUpdate;
        config.funcs.OnRender = CharmApp::OnRender;
        config.funcs.OnRenderUI = CharmApp::OnRenderUI;
        config.funcs.OnShutdown = CharmApp::OnShutdown;

        Application::Setup(config);
        Application::Run();
        Application::Shutdown();
    }
}
