#include "CharmApp.h"
#include "CharmHub.h"
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

        state.project = CharmHub::GetProject();

        ProjectSerializer::SetContext(&state.project);
        SceneSerializer::SetContext(&state.editorScene);
        SceneHeirarchyPanel::SetContext(state.editorScene);
        ContentBrowserPanel::Init();
        ToolbarPanel::Init();

        const std::filesystem::path scriptModulePath = ProjectManager::GetAssetFileSystemPath(state.project.scriptModulePath, state.project);
        if (std::filesystem::exists(scriptModulePath))
            ScriptManager::LoadModule(scriptModulePath.c_str());

        if (!state.project.startScenePath.empty())
        {
            std::filesystem::path scenePath = ProjectManager::GetAssetFileSystemPath(state.project.startScenePath, state.project);
            OpenScene(scenePath.c_str());
        }

        FileDialogs::SetDefaultPath(ProjectManager::GetAssetPath(state.project));
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
            OnSceneNew();

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
                    SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
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

        SceneHeirarchyPanel::SetContext(*state.activeScene);
        Scenes::AlignParentsAndChildren(*state.activeScene);
        Scenes::OnRuntimeStart(*state.activeScene);

        if (prev.IsHandleValid())
        {
            auto& prevInternal = prev.GetComponent<InternalComponent>();
            SceneHeirarchyPanel::SetSelectedEntity(Entities::FindWithUUID(prevInternal.id, state.activeScene));
        }
    }

    void OnSceneStop()
    {
        Entity prev = SceneHeirarchyPanel::GetSelectedEntity();

        SceneHeirarchyPanel::SetContext(state.editorScene);
        state.sceneState = SceneState::Editor;
        state.activeScene = &state.editorScene;

        if (prev.IsHandleValid())
        {
            auto& prevInternal = prev.GetComponent<InternalComponent>();
            SceneHeirarchyPanel::SetSelectedEntity(Entities::FindWithUUID(prevInternal.id, state.activeScene));
        }

        Scenes::OnRuntimeStop(state.runtimeScene);
        state.runtimeScene = Scenes::Create();
    }

    void OnSceneNew()
    {
        if (state.sceneState != SceneState::Editor)
            return;

        SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
        AssetManager::Clean();
        ScriptManager::ClearBindings();
        Scenes::ClearRegistry(state.editorScene);
    }

    void OnSceneOpen()
    {
        if (state.sceneState != SceneState::Editor)
            Scenes::OnRuntimeStop(*state.activeScene);

        if (FileDialogs::Open())
        {
            std::string path = FileDialogs::GetSelectedPath().string();
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

        SceneSerializer::SetContext(&state.editorScene);
        SceneSerializer::Serialize(state.currentScenePath.c_str());
        INFO("Saved scene %s", state.currentScenePath.c_str());
    }

    void OnSceneSaveAs()
    {
        if (FileDialogs::Save())
        {
            std::string path = FileDialogs::GetSelectedPath().string();
            SceneSerializer::SetContext(&state.editorScene);
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
        OnSceneNew();

        SceneSerializer::SetContext(&state.editorScene);
        SceneSerializer::Deserialize(path);
        state.currentScenePath = path;
        ScriptManager::ReloadModule();

        if (Entities::FindWithTag("Main Camera", state.activeScene) == Entity_Null)
        {
            auto& config = Application::GetConfig();
            Entity mainCamera = Scenes::CreateEntity(state.editorScene, "Main Camera");
            auto& cameraComponent = mainCamera.AddComponent<Camera2DComponent>();
            cameraComponent.camera.offset.x = (float)config.virtualWidth / (float)Application::GetPixelsPerUnit() / 2.f;
            cameraComponent.camera.offset.y = (float)config.virtualHeight / (float)Application::GetPixelsPerUnit() / 2.f;
            cameraComponent.isPrimary = true;
        }

        if (std::filesystem::exists(state.currentScenePath))
        {
            const std::filesystem::path homeDirectory = Utils::GetHomeDirectory();
            const std::filesystem::path absolutePath = homeDirectory / std::filesystem::relative(path, homeDirectory);
            INFO("Loaded scene %s", absolutePath.c_str());
        }
    }

    s32 GetPixelData() { return state.pixelData; }
    Scene* GetActiveScene() { return state.activeScene; }
    SceneState GetActiveSceneState() { return state.sceneState; }
    Project& GetProject() { return state.project; }
}
