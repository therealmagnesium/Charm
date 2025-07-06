#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

namespace CharmApp
{
    struct CharmState
    {
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
    void OnSceneSaveAs();
    void OnDuplicateEntity();

    s32 GetPixelData();
    Scene* GetActiveScene();
    SceneState GetActiveSceneState();
}
