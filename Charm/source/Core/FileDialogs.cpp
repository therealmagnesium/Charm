#include "Core/FileDialogs.h"
#include "Core/Log.h"

#include "Graphics/Window.h"

#include <nfd.hpp>
#include <nfd_sdl3.h>

using namespace Charm::Graphics;

namespace Charm
{
    namespace Core
    {
        static FileDialogState state;
        static bool isInitialized = false;

        namespace FileDialogs
        {
            void Init()
            {
                if (isInitialized)
                {
                    WARN("NFD cannot be initialized more than once");
                    return;
                }

                if (!NFD::Init())
                {
                    ERROR("FileDialogs::Init - Failed to initialize NFD!");
                    return;
                }

                state.defaultPath = "assets";
                state.selectedPaths.resize(1);
                INFO("NFD was initialized successfully");
            }

            void Shutdown()
            {
                INFO("NFD is shutting down...");
                state.selectedPaths.clear();
                NFD::Quit();
            }

            bool Open()
            {
                void* windowHandle = Window::GetHandle();

                char* outPath = NULL;

                nfdwindowhandle_t parentWindow;
                NFD_GetNativeWindowFromSDLWindow((SDL_Window*)windowHandle, &parentWindow);

                nfdresult_t result = NFD::OpenDialog(outPath, NULL, 0, state.defaultPath.c_str(), parentWindow);
                if (result == NFD_CANCEL || result == NFD_ERROR)
                    return false;

                state.selectedPaths[0] = outPath;
                free(outPath);

                return true;
            }

            bool OpenMultiple() { ASSERT(false, "FileDialogs::OpenMultiple - No implementation!"); }

            bool Save()
            {
                void* windowHandle = Window::GetHandle();

                char* outPath = NULL;

                nfdwindowhandle_t parentWindow;
                NFD_GetNativeWindowFromSDLWindow((SDL_Window*)windowHandle, &parentWindow);

                nfdresult_t result = NFD::SaveDialog(outPath, NULL, 0, state.defaultPath.c_str(), NULL, parentWindow);
                if (result == NFD_CANCEL || result == NFD_ERROR)
                    return false;

                state.selectedPaths[0] = outPath;
                free(outPath);

                return true;
            }

            const std::filesystem::path& GetSelectedPath() { return state.selectedPaths[0]; }
            const std::vector<std::filesystem::path>& GetSelectedPathMulti() { return state.selectedPaths; }

            void SetDefaultPath(const std::filesystem::path& path) { state.defaultPath = path; }
        }
    }
}
