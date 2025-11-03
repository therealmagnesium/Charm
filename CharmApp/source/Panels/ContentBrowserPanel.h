#pragma once
#include <Graphics/Texture.h>
#include <filesystem>

using namespace Charm;

namespace CharmApp
{
    struct ContentBrowserRenameState
    {
        char fileName[256];
        std::filesystem::path path;
        bool isDirectory = false;
        bool isActive = false;
    };

    struct ContentBrowserState
    {
        float padding = 64.f;
        float thumbnailSize = 64.f;
        u32 columnCount = 0;
        ContentBrowserRenameState rename;
        std::filesystem::path currentDirectory;
        std::filesystem::path selectedFilePath;
        std::filesystem::path homeDirectory;
        Graphics::Texture iconFile;
        Graphics::Texture iconFolder;
    };

    namespace ContentBrowserPanel
    {
        void Init();
        void Shutdown();
        void Display();

        const std::filesystem::path& GetSelectedFilePath();
        void ClearSelectedFilePath();
    }
}
