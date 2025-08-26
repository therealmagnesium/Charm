#include "ECS/ScriptManager.h"
#include <dlfcn.h>

namespace Charm
{
    namespace ECS
    {
        static ScriptManagerState state;

        namespace ScriptManager
        {
            void LoadModule(const char* path)
            {
                state.modulePath = path;
                state.moduleHandle = dlopen(path, RTLD_NOW);
                ASSERT_ERROR(state.moduleHandle != NULL, "ScriptManager::LoadModule - Failed to load module \"%s\"!", dlerror());

                state.RegisterScripts = (ScriptRegisterFunc)dlsym(state.moduleHandle, "RegisterScripts");
                ASSERT_ERROR(state.RegisterScripts != NULL, "ScriptManager::LoadModule - Failed to find function \"RegisterScripts\"!");

                state.RegisterScripts();

                INFO("Script Manager successfully loaded module \"%s\"", path);
            }

            void UnloadModule()
            {
                if (state.moduleHandle != NULL)
                {
                    INFO("Script Manager is unloading module %s...", state.modulePath.c_str());
                    dlclose(state.moduleHandle);
                    state.moduleHandle = NULL;
                    state.RegisterScripts = NULL;
                    state.bindings.clear();
                }
            }

            void ReloadModule()
            {
                std::string pathString = state.modulePath.string();
                UnloadModule();
                LoadModule(pathString.c_str());
            }

            void ClearBindings() { state.bindings.clear(); }

            ScriptInitFunc GetScriptInitFunc(const std::string& name)
            {
                auto it = state.bindings.find(name);
                return (it != state.bindings.end()) ? state.bindings[name].CreateScript : NULL;
            }

            ScriptShutdownFunc GetScriptDestroyFunc(const std::string& name)
            {
                auto it = state.bindings.find(name);
                return (it != state.bindings.end()) ? state.bindings[name].DestroyScript : NULL;
            }

            ScriptManagerState& GetState() { return state; }
            const ScriptBindingMap& GetAllBindings() { return state.bindings; }

        }
    }
}
