#include "CharmApp.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/DebugStatsPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/SceneHeirarchyPanel.h"
#include "Panels/SceneViewport.h"
#include "Panels/ToolbarPanel.h"

#include <Charm.h>
#include <imgui.h>
#include <glad/glad.h>

using namespace Charm;
using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

namespace CharmApp
{
    static CharmState state;

    void OnCreate()
    {
        const ApplicationConfig& config = Application::GetConfig();
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);

        FramebufferSpecification framebufferSpec;
        framebufferSpec.width = config.virtualWidth;
        framebufferSpec.height = config.virtualHeight;
        framebufferSpec.attachments = {TextureFormat::RGBA, TextureFormat::RedInteger, TextureFormat::DepthStencil};
        state.framebuffer = Framebuffers::Create(framebufferSpec);

        state.editorScene = Scenes::Create();
        state.runtimeScene = Scenes::Create();
        state.activeScene = &state.editorScene;

        state.project = ProjectManager::Load("SandboxProject/Sandbox.chprj");
        if (!state.project.startScenePath.empty())
        {
            std::filesystem::path scenePath = ProjectManager::GetAssetFileSystemPath(state.project.startScenePath, state.project);
            OpenScene(scenePath.c_str());
        }
        FileDialogs::SetDefaultPath(ProjectManager::GetAssetPath(state.project));

        SceneSerializer::SetContext(state.editorScene);
        SceneHeirarchyPanel::SetContext(state.editorScene);
        ContentBrowserPanel::Init();
        ToolbarPanel::Init();

        const std::filesystem::path scriptModulePath = ProjectManager::GetAssetFileSystemPath("scripts/binaries/libCharmScriptModule.so", state.project);
        ScriptManager::LoadModule(scriptModulePath.c_str());
    }

    void OnUpdate()
    {
        ASSERT(state.activeScene != NULL, "CharmApp::OnUpdate - The currently active scene is null!");
        Input::Capture(true);

        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        if (Input::IsKeyPressed(KEY_F1))
            state.activeScene->isDebugRenderingEnabled = !state.activeScene->isDebugRenderingEnabled;

        if (Input::IsKeyPressed(KEY_F2))
            Scenes::ResetEditorCameras(*state.activeScene);

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_R))
            ScriptManager::ReloadModule();

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_N))
            OnSceneNew(true);

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && !Input::IsKeyDown(KEY_LEFT_SHIFT) && Input::IsKeyPressed(KEY_S))
            OnSceneSave();

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyDown(KEY_LEFT_SHIFT) && Input::IsKeyPressed(KEY_S))
            OnSceneSaveAs();

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_O))
            OnSceneOpen();

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_D))
            OnDuplicateEntity();

        Input::Capture(SceneViewportPanel::IsFocused());
        Application::SetViewportPosition(SceneViewportPanel::GetPosition());
        Application::SetViewportSize(SceneViewportPanel::GetSize());

        if (state.sceneState == SceneState::Editor)
        {
            if (Input::IsMouseClicked(MOUSE_BUTTON_LEFT) && !Input::IsKeyDown(KEY_LEFT_ALT))
            {
                const glm::vec2 glViewportMouse = Utils::ScreenToViewportGL(Input::GetMousePosition(),
                                                                            SceneViewportPanel::GetPosition(),
                                                                            SceneViewportPanel::GetSize());

                Framebuffers::Bind(state.framebuffer);
                state.pixelData = Framebuffers::ReadPixel(state.framebuffer, 1, (u32)glViewportMouse.x, (u32)glViewportMouse.y);
                Framebuffers::Unbind();

                if (state.pixelData != -1)
                {
                    Entity entity = Entities::Create((entt::entity)state.pixelData, &state.editorScene);
                    SceneHeirarchyPanel::SetSelectedEntity(entity);
                }
                else if (state.pixelData == -1 && SceneViewportPanel::IsFocused() && SceneViewportPanel::IsHovered())
                    SceneHeirarchyPanel::SetSelectedEntity((Entity){});
            }

            Scenes::UpdateEditor(*state.activeScene);
        }
        else
            Scenes::UpdateRuntime(*state.activeScene);
    }

    void OnRender()
    {
        Framebuffers::Bind(state.framebuffer);
        RenderCommand::Clear();
        Framebuffers::ClearAttachment(state.framebuffer, 1, -1);

        if (state.sceneState == SceneState::Editor)
            Scenes::RenderEditor(*state.activeScene, SceneHeirarchyPanel::GetSelectedEntity());
        else
            Scenes::RenderRuntime(*state.activeScene);

        Framebuffers::Unbind();
    }

    void OnRenderUI()
    {
        ImGui::DockSpaceOverViewport();

        AssetRegistryPanel::Display();
        ContentBrowserPanel::Display();
        SceneHeirarchyPanel::Display();
        InspectorPanel::Display();
        SceneViewportPanel::Display(state.framebuffer.colorAttachments[0]);
        DebugStatsPanel::Display();
        ToolbarPanel::Display();
    }

    void OnShutdown()
    {
        ScriptManager::UnloadModule();
        AssetManager::Clean();
        ContentBrowserPanel::Shutdown();
        ToolbarPanel::Shutdown();
        Framebuffers::Destroy(state.framebuffer);
    }

    void OnScenePlay()
    {
        Entity prev = SceneHeirarchyPanel::GetSelectedEntity();

        state.sceneState = SceneState::Runtime;
        state.runtimeScene = Scenes::Copy(state.editorScene);
        state.activeScene = &state.runtimeScene;
        Scenes::OnRuntimeStart(*state.activeScene);
        SceneHeirarchyPanel::SetContext(*state.activeScene);

        if (prev)
        {
            auto& prevInternal = prev.GetComponent<InternalComponent>();
            SceneHeirarchyPanel::SetSelectedEntity(Entities::FindWithTag(prevInternal.tag.c_str(), state.activeScene));
        }
    }

    void OnSceneStop()
    {
        Entity prev = SceneHeirarchyPanel::GetSelectedEntity();
        const char* prevTag = "";

        if (prev)
        {
            auto& prevInternal = prev.GetComponent<InternalComponent>();
            prevTag = prevInternal.tag.c_str();
        }

        Scenes::OnRuntimeStop(*state.activeScene);
        SceneHeirarchyPanel::SetContext(state.editorScene);
        state.sceneState = SceneState::Editor;
        state.activeScene = &state.editorScene;

        if (prev)
            SceneHeirarchyPanel::SetSelectedEntity(Entities::FindWithTag(prevTag, state.activeScene));

        state.runtimeScene = Scenes::Create();
    }

    void OnSceneNew(bool shouldCreateMainCamera)
    {
        if (state.sceneState != SceneState::Editor)
            return;

        SceneHeirarchyPanel::SetSelectedEntity((Entity){});
        AssetManager::Clean();
        ScriptManager::ClearBindings();
        Scenes::ClearRegistry(state.editorScene);

        if (shouldCreateMainCamera)
        {
            auto& config = Application::GetConfig();
            Entity mainCamera = Scenes::CreateEntity(state.editorScene, "Main Camera");
            auto& cameraComponent = mainCamera.AddComponent<Camera2DComponent>();
            cameraComponent.camera.offset.x = (float)config.virtualWidth / (float)Application::GetPixelsPerUnit() / 2.f;
            cameraComponent.camera.offset.y = (float)config.virtualHeight / (float)Application::GetPixelsPerUnit() / 2.f;
            cameraComponent.isPrimary = true;
        }
    }

    void OnSceneOpen()
    {
        if (state.sceneState != SceneState::Editor)
            Scenes::OnRuntimeStop(*state.activeScene);

        if (FileDialogs::Open())
        {
            const std::string& path = FileDialogs::GetSelectedPath();
            CharmApp::OpenScene(path.c_str());
        }
    }

    void OnSceneSave()
    {
        if (state.currentScenePath.empty())
        {
            OnSceneSaveAs();
            return;
        }

        SceneSerializer::SetContext(state.editorScene);
        SceneSerializer::Serialize(state.currentScenePath.c_str());
        INFO("Saved scene %s", state.currentScenePath.c_str());
    }

    void OnSceneSaveAs()
    {
        if (FileDialogs::Save())
        {
            const std::string& path = FileDialogs::GetSelectedPath();
            SceneSerializer::SetContext(state.editorScene);
            SceneSerializer::Serialize(path.c_str());
            INFO("Saved scene to %s", path.c_str());
        }
    }

    void OnDuplicateEntity()
    {
        if (state.sceneState != SceneState::Editor)
            return;

        Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();
        if (selectedEntity)
        {
            Entity duplicate = Scenes::DuplicateEntity(state.editorScene, selectedEntity);
            SceneHeirarchyPanel::SetSelectedEntity(duplicate);
        }
    }

    void OpenScene(const char* path)
    {
        OnSceneNew(false);

        Scene newScene = Scenes::Create();
        SceneSerializer::SetContext(newScene);
        SceneSerializer::Deserialize(path);

        state.currentScenePath = path;

        state.editorScene = Scenes::Copy(newScene);
        Scenes::ClearRegistry(newScene);
        SceneSerializer::SetContext(state.editorScene);

        INFO("Loaded scene %s", path);
    }

    s32 GetPixelData() { return state.pixelData; }
    Scene* GetActiveScene() { return state.activeScene; }
    SceneState GetActiveSceneState() { return state.sceneState; }
    Project& GetProject() { return state.project; }
}
