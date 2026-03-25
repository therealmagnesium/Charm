#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

using namespace Charm::Projects;

namespace CharmApp
{
    struct CharmState
    {
        std::filesystem::path currentScenePath;
        Project project;
        Timer timer;
        Framebuffer framebuffer;
        Framebuffer framebuffersBloom[2];
        SceneState sceneState;
        ShadowMap shadowMap;
        Scene editorScene;
        Scene runtimeScene;
        Scene* activeScene = NULL;
        s32 pixelData = -1;
        bool showPreferencesWindow = false;
        bool isBloomPassHorizontal = true;
        bool isViewportMaximized = false;
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

    bool IsBloomPassHorizontal();
    s32 GetPixelData();
    Scene* GetActiveScene();
    SceneState GetActiveSceneState();
    Project& GetProject();
    Framebuffer& GetFramebufferHDR();
    Framebuffer& GetFramebufferBloom(bool horizontalPass);

    void SetPixelData(s32 data);
}
