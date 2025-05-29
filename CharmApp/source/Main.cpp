#include <Core/Application.h>
#include <Core/Log.h>

using namespace Charm::Core;

int main(int argc, char** argv)
{
    ApplicationConfig config;
    config.name = "Charm Framework";
    config.author = "Magnus Ahlstromer V";
    config.funcs.OnCreate = []() {};
    config.funcs.OnUpdate = []() {};
    config.funcs.OnRender = []() {};
    config.funcs.OnRenderUI = []() {};
    config.funcs.OnShutdown = []() {};

    Application::Setup(config);
    Application::Run();
    Application::Shutdown();
}
