#pragma once
#include "ECS/Entity.h"
#include "Core/Log.h"
#include <string>
#include <filesystem>

namespace Charm
{
    namespace ECS
    {
        using ScriptInitFunc = Scriptable* (*)();
        using ScriptShutdownFunc = void (*)(Scriptable*);
        using ScriptRegisterFunc = void (*)();
        using ScriptBindingMap = std::unordered_map<std::string, struct NativeScriptBinding>;

        struct NativeScriptBinding
        {
            ScriptInitFunc CreateScript;
            ScriptShutdownFunc DestroyScript;
        };

        struct ScriptManagerState
        {
            void* moduleHandle = NULL;
            std::filesystem::path modulePath;
            ScriptBindingMap bindings;

            ScriptRegisterFunc RegisterScripts = NULL;
        };

        namespace ScriptManager
        {
            void LoadModule(const char* path);
            void UnloadModule();
            void ReloadModule();
            void ClearBindings();

            bool IsModuleLoaded();
            ScriptInitFunc GetScriptInitFunc(const std::string& name);
            ScriptShutdownFunc GetScriptDestroyFunc(const std::string& name);
            ScriptManagerState& GetState();
            const ScriptBindingMap& GetAllBindings();

            template <typename T>
            inline void BindScript(const char* name)
            {
                bool deriveCheck = std::is_base_of_v<Scriptable, T>;
                ASSERT(deriveCheck, "ScriptManager::Bind - Script must derive from \"Scriptable\" in order to bind");

                ScriptManagerState& state = GetState();
                state.bindings[name].CreateScript = []() {Scriptable* instance = new T; return instance; };
                state.bindings[name].DestroyScript = [](Scriptable* instance) {delete instance; instance = NULL; };
            }
        }
    }
}
