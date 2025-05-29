#include "Core/Application.h"
#include "Core/Log.h"

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

                state.isRunning = true;
                isInitialized = true;

                INFO("Application \"%s\" was initiallized successfully", state.config.name.c_str());
            }

            void Shutdown()
            {
                INFO("Shutting down the application...");
            }

            void Run()
            {
                state.config.funcs.OnCreate();

                while (state.isRunning)
                {
                    /*
                    state.config.funcs.OnUpdate();
                    state.config.funcs.OnRender();
                    state.config.funcs.OnRenderUI();*/
                }

                state.config.funcs.OnShutdown();
            }

            bool IsRunning() { return state.isRunning; }
            ApplicationConfig& GetConfig() { return state.config; }
        }
    }
}
