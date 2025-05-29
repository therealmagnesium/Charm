#pragma once
#include "Core/Base.h"
#include <string>

namespace Charm
{
    namespace Core
    {
        typedef void (*AppFunc)(void);

        struct ApplicationImplementation
        {
            AppFunc OnCreate = NULL;
            AppFunc OnUpdate = NULL;
            AppFunc OnRender = NULL;
            AppFunc OnRenderUI = NULL;
            AppFunc OnShutdown = NULL;
        };

        struct ApplicationConfig
        {
            std::string name = "Untitled";
            std::string author = "None specified";
            u32 virtualWidth = 1280;
            u32 virtualHeight = 720;
            ApplicationImplementation funcs;
        };

        struct ApplicationState
        {
            bool isRunning = false;
            ApplicationConfig config;
        };

        namespace Application
        {
            void Setup(const ApplicationConfig& config);
            void Shutdown();
            void Run();
            void Quit();

            bool IsRunning();
            ApplicationConfig& GetConfig();
        }
    }
}
