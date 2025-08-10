#pragma once
#include <Graphics/Texture.h>
#include <filesystem>

using namespace Charm;

namespace CharmApp
{
    struct ContentBrowserState
    {
        std::filesystem::path currentDirectory;
        std::filesystem::path selectedFilePath;
        Graphics::Texture iconFile;
        Graphics::Texture iconFolder;
        float padding = 64.f;
        float thumbnailSize = 64.f;
    };

    namespace ContentBrowserPanel
    {
        void Init();
        void Shutdown();
        void Display();

        std::filesystem::path& GetSelectedFilePath();
    }
}
