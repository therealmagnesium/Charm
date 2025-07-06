#include "CharmApp.h"
#include "Panels/DebugStatsPanel.h"
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

        SceneSerializer::SetContext(state.editorScene);
        SceneHeirarchyPanel::SetContext(state.editorScene);
        ToolbarPanel::Init();
    }

    void OnUpdate()
    {
        Input::Capture(true);

        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        if (Input::IsKeyPressed(KEY_F2))
            Scenes::ResetEditorCameras(*state.activeScene);

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_N))
            OnSceneNew();

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_S))
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
                    Scene* currentScene = SceneHeirarchyPanel::GetContext();
                    Entity entity = Entities::Create((entt::entity)state.pixelData, currentScene);
                    SceneHeirarchyPanel::SetSelectedEntity(entity);
                }
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
            Scenes::RenderEditor(*state.activeScene);
        else
            Scenes::RenderRuntime(*state.activeScene);

        Framebuffers::Unbind();
    }

    void OnRenderUI()
    {
        ImGui::DockSpaceOverViewport();

        SceneHeirarchyPanel::Display();
        SceneViewportPanel::Display(state.framebuffer.colorAttachments[0]);
        DebugStatsPanel::Display();
        ToolbarPanel::Display();

        ImGui::Begin("Asset Registry");

        for (auto& [handle, metadata] : AssetManager::GetRegistry())
        {
            ImGui::Text("Handle: 0x%lx", handle);
            ImGui::Text("Path: %s", metadata.path.c_str());
            ImGui::Text("Type: %s", Utils::AssetTypeToString(metadata.type).c_str());
        }

        ImGui::End();
    }

    void OnShutdown()
    {
        AssetManager::Clean();
        ToolbarPanel::Shutdown();
        Framebuffers::Destroy(state.framebuffer);
    }

    void OnScenePlay()
    {
        state.sceneState = SceneState::Runtime;
        state.runtimeScene = Scenes::Copy(state.editorScene);
        state.activeScene = &state.runtimeScene;
        Scenes::OnRuntimeStart(*state.activeScene);
        SceneHeirarchyPanel::SetContext(*state.activeScene);
    }

    void OnSceneStop()
    {
        Scenes::OnRuntimeStop(*state.activeScene);
        state.sceneState = SceneState::Editor;
        state.activeScene = &state.editorScene;
        state.runtimeScene = (Scene){};
        SceneHeirarchyPanel::SetContext(*state.activeScene);
    }

    void OnSceneNew()
    {
        if (state.sceneState != SceneState::Editor)
            return;

        Scenes::ClearRegistry(state.editorScene);
        SceneHeirarchyPanel::SetSelectedEntity((Entity){});
        AssetManager::Clean();
    }

    void OnSceneOpen()
    {
        if (state.sceneState != SceneState::Editor)
            Scenes::OnRuntimeStop(*state.activeScene);

        if (FileDialogs::Open())
        {
            OnSceneNew();

            const std::string& path = FileDialogs::GetSelectedPath();
            SceneSerializer::Deserialize(path.c_str());
            INFO("Loaded scene %s", path.c_str());
        }
    }

    void OnSceneSaveAs()
    {
        if (FileDialogs::Save())
        {
            const std::string& path = FileDialogs::GetSelectedPath();
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

    s32 GetPixelData() { return state.pixelData; }
    Scene* GetActiveScene() { return state.activeScene; }
    SceneState GetActiveSceneState() { return state.sceneState; }
}
