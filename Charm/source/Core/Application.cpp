#include "Core/Application.h"
#include "Core/AssetManager.h"
#include "Core/FileDialogs.h"
#include "Core/Input.h"
#include "Core/Log.h"
#include "Core/Random.h"
#include "Core/Time.h"

#include "Graphics/Renderer.h"
#include "Graphics/RenderCommand.h"
#include "Graphics/Window.h"

#include "UI/UI.h"

using namespace Charm::Graphics;

namespace Charm
{
    namespace Core
    {
        static ApplicationState state;
        static bool isInitialized = false;

        namespace Application
        {
            void Setup(const ApplicationConfig& config)
            {
                if (isInitialized)
                {
                    WARN("Cannot initialize the application more than once");
                    return;
                }

                state.config = config;

                ASSERT(state.config.funcs.OnCreate != NULL &&
                           state.config.funcs.OnUpdate != NULL &&
                           state.config.funcs.OnRender != NULL &&
                           state.config.funcs.OnRenderUI != NULL &&
                           state.config.funcs.OnShutdown != NULL,
                       "The application's implementation hasn't been defined!");

                printf("============================================ Core Program Begins "
                       "=============================================\n");

                Random::Init();
                Input::Initialize();
                Time::Initialize(60);
                Renderer::Initialize();
                FileDialogs::Init();
                UI::SetupContext();
                AssetManager::Init(&state.assets);

                state.isRunning = true;
                isInitialized = true;
                INFO("Application \"%s\" was initiallized successfully", state.config.name.c_str());
                printf("============================================ Client Program Begins "
                       "=============================================\n");
            }

            void Shutdown()
            {
                INFO("Application \"%s\" is shutting down...", state.config.name.c_str());
                FileDialogs::Shutdown();
                UI::DestroyContext();
                Renderer::Shutdown();
            }

            void Run()
            {
                state.config.funcs.OnCreate();

                while (state.isRunning)
                {
                    Time::Update();
                    Window::HandleEvents();
                    state.config.funcs.OnUpdate();

                    RenderCommand::Clear();

                    state.config.funcs.OnRender();

                    UI::BeginFrome();
                    state.config.funcs.OnRenderUI();
                    UI::EndFrame();

                    UI::Display();
                    Window::Display();
                }

                printf("============================================ Client Program Ends "
                       "=============================================\n");

                state.config.funcs.OnShutdown();

                printf("============================================ Core Program Ends "
                       "=============================================\n");
            }

            void Quit()
            {
                state.isRunning = false;
            }

            bool IsRunning() { return state.isRunning; }
            const ApplicationConfig& GetConfig() { return state.config; }
            const glm::vec2& GetViewportPosition() { return state.viewportPosition; }
            const glm::vec2& GetViewportSize() { return state.viewportSize; }

            void SetViewportPosition(const glm::vec2& position) { state.viewportPosition = position; }
            void SetViewportSize(const glm::vec2& size) { state.viewportSize = size; }
        }
    }
}
