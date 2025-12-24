#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

using namespace Charm::Projects;

namespace CharmApp
{
    struct CharmState
    {
        Project project;
        Timer timer;
        Framebuffer framebuffer;
        SceneState sceneState;
        Scene editorScene;
        Scene runtimeScene;
        Scene* activeScene = NULL;
        s32 pixelData = -1;
        bool showPreferencesWindow = false;
        std::filesystem::path currentScenePath;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();

    void OnScenePlay();
    void OnSceneStop();
    void OnSceneNew();
    void OnSceneOpen();
    void OnSceneSave();
    void OnSceneSaveAs();
    void OnDuplicateEntity();
    void OnProjectOpen();

    void OpenScene(const std::filesystem::path& path);

    void DrawMenuBar();
    void DrawPreferencesMenu();

    s32 GetPixelData();
    Scene* GetActiveScene();
    SceneState GetActiveSceneState();
    Project& GetProject();

    void SetPixelData(s32 data);
}
