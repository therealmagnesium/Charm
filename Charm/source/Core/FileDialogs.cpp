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
            std::filesystem::path EnsureExtension(const std::filesystem::path& path, const char* extension);

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

                isInitialized = false;
            }

            bool Open(FileDialogFilter* filters, u8 filterCount)
            {
                void* windowHandle = Window::GetHandle();
                char* outPath = NULL;

                nfdwindowhandle_t parentWindow;
                NFD_GetNativeWindowFromSDLWindow((SDL_Window*)windowHandle, &parentWindow);

                nfdresult_t result = NFD::OpenDialog(outPath, (nfdfilteritem_t*)filters, filterCount, state.defaultPath.c_str(), parentWindow);
                if (result == NFD_CANCEL || result == NFD_ERROR)
                    return false;

                state.selectedPaths[0] = outPath;
                free(outPath);

                return true;
            }

            bool OpenMultiple() { ASSERT(false, "FileDialogs::OpenMultiple - No implementation!"); }

            bool Save(FileDialogFilter* filters, u8 filterCount)
            {
                void* windowHandle = Window::GetHandle();

                char* outPath = NULL;

                nfdwindowhandle_t parentWindow;
                NFD_GetNativeWindowFromSDLWindow((SDL_Window*)windowHandle, &parentWindow);

                nfdresult_t result = NFD::SaveDialog(outPath, (nfdfilteritem_t*)filters, filterCount, state.defaultPath.c_str(), NULL, parentWindow);
                if (result == NFD_CANCEL || result == NFD_ERROR)
                    return false;

                std::filesystem::path selectedPath = outPath;
                free(outPath);

                // Ensure the path has the correct extension based on the first filter
                if (filters != NULL && filterCount > 0)
                {
                    // Get the first extension from the specification (e.g., "anim" from "anim")
                    // If there are multiple extensions (e.g., "png,jpg,jpeg"), take the first one
                    std::string spec = filters[0].specification;
                    u64 commaPos = spec.find(',');
                    std::string firstExtension = (commaPos != std::string::npos) ? spec.substr(0, commaPos) : spec;

                    selectedPath = EnsureExtension(selectedPath, firstExtension.c_str());
                }

                state.selectedPaths[0] = selectedPath;
                return true;
            }

            std::filesystem::path GetSelectedPath() { return state.selectedPaths[0]; }
            const std::vector<std::filesystem::path>& GetSelectedPathMulti() { return state.selectedPaths; }

            void SetDefaultPath(const std::filesystem::path& path) { state.defaultPath = path; }

            std::filesystem::path EnsureExtension(const std::filesystem::path& path, const char* extension)
            {
                std::filesystem::path resultPath = path;

                // Check if the path already has an extension
                if (resultPath.has_extension())
                {
                    std::string currentExtension = resultPath.extension().string();
                    // Remove the leading dot from the extension for comparison
                    if (!currentExtension.empty() && currentExtension[0] == '.')
                        currentExtension = currentExtension.substr(1);

                    // If it matches the expected extension, we're good
                    if (currentExtension == extension)
                        return resultPath;
                }

                // Either no extension or wrong extension - append the correct one
                resultPath += ".";
                resultPath += extension;
                return resultPath;
            }
        }
    }
}
