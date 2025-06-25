#pragma once
#include "Core/AssetManager.h"
#include "Core/Base.h"

#include <glm/glm.hpp>
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
            s32 virtualWidth = 1280;
            s32 virtualHeight = 720;
            ApplicationImplementation funcs;
        };

        struct ApplicationState
        {
            bool isRunning = false;
            glm::vec2 viewportPosition;
            glm::vec2 viewportSize;
            ApplicationConfig config;
            AssetCollection assets;
        };

        namespace Application
        {
            void Setup(const ApplicationConfig& config);
            void Shutdown();
            void Run();
            void Quit();

            bool IsRunning();
            const ApplicationConfig& GetConfig();
            const glm::vec2& GetViewportPosition();
            const glm::vec2& GetViewportSize();

            void SetViewportPosition(const glm::vec2& position);
            void SetViewportSize(const glm::vec2& size);
        }
    }
}
