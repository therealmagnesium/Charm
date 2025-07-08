#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

namespace CharmApp
{
    struct CharmState
    {
        std::string currentScenePath;
        Framebuffer framebuffer;
        SceneState sceneState;
        Scene editorScene;
        Scene runtimeScene;
        Scene* activeScene = NULL;
        s32 pixelData = -1;
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
}
