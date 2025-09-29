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
        Framebuffer framebuffer;
        SceneState sceneState;
        Scene editorScene;
        Scene runtimeScene;
        Scene* activeScene = NULL;
        s32 pixelData = -1;
        std::string currentScenePath;
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

    void OpenScene(const char* path);

    s32 GetPixelData();
    Scene* GetActiveScene();
    SceneState GetActiveSceneState();
    Project& GetProject();

    void SetPixelData(s32 data);
}
