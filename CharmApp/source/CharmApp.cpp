#include "CharmApp.h"
#include "CharmHub.h"
#include "Panels/AnimationPanel.h"
#include "Panels/AssetRegistryPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/DebugStatsPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/SceneHeirarchyPanel.h"
#include "Panels/SceneViewport.h"
#include "Panels/TextureSlicerPanel.h"
#include "Panels/TilePalettePanel.h"
#include "Panels/ToolbarPanel.h"

#include <Charm.h>
#include <imgui.h>
#include <ImGuizmo.h>
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
        AssetRegistryPanel::Init();
        ContentBrowserPanel::Init();
        ToolbarPanel::Init();
        AnimationPanel::Init();

        const std::filesystem::path scriptModulePath = ProjectManager::GetScriptModulePath(state.project);
        if (std::filesystem::exists(scriptModulePath))
            ScriptManager::LoadModule(scriptModulePath);

        if (!state.project.startScenePath.empty())
        {
            std::filesystem::path scenePath = ProjectManager::GetStartScenePath(state.project);
            OpenScene(scenePath.c_str());
        }

        FileDialogs::SetDefaultPath(ProjectManager::GetAssetPath(state.project));
        Time::StartTimer(state.timer);
    }

    void OnShutdown()
    {
        ScriptManager::UnloadModule();
        AssetManager::Clean();
        ContentBrowserPanel::Shutdown();
        ToolbarPanel::Shutdown();
        AnimationPanel::Shutdown();
        Framebuffers::Destroy(state.framebuffer);
    }

    void OnUpdate()
    {
        ASSERT(state.activeScene != NULL, "CharmApp::OnUpdate - The currently active scene is null!");
        Time::UpdateTimer(state.timer);

        Input::Capture(true);

        if (Input::IsKeyPressed(KEY_F1))
            state.activeScene->isDebugRenderingEnabled = !state.activeScene->isDebugRenderingEnabled;

        if (state.sceneState == SceneState::Editor)
        {
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

            if (Input::IsKeyDown(KEY_LEFT_CTRL) && !Input::IsKeyDown(KEY_LEFT_SHIFT) && Input::IsKeyPressed(KEY_O))
                OnSceneOpen();

            if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyDown(KEY_LEFT_SHIFT) && Input::IsKeyPressed(KEY_O))
                OnProjectOpen();

            if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_D))
                OnDuplicateEntity();

            if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyDown(KEY_LEFT_SHIFT) && Input::IsKeyPressed(KEY_P))
                state.showPreferencesWindow = !state.showPreferencesWindow;

            Input::Capture(SceneViewportPanel::IsFocused());
            if (Input::IsKeyPressed(KEY_E))
                ToolbarPanel::SetManipulationType(ImGuizmo::OPERATION::SCALE);
            if (Input::IsKeyPressed(KEY_R))
                ToolbarPanel::SetManipulationType(ImGuizmo::OPERATION::ROTATE);
            if (Input::IsKeyPressed(KEY_T))
                ToolbarPanel::SetManipulationType(ImGuizmo::OPERATION::TRANSLATE);
            if (Input::IsKeyPressed(KEY_G))
                state.project.grid.isEnabled = !state.project.grid.isEnabled;

            if (Input::IsKeyPressed(KEY_F))
            {
                Entity& entity = SceneHeirarchyPanel::GetSelectedEntity();
                if (entity.IsHandleValid())
                {
                    const auto& entityTransform = entity.GetComponent<TransformComponent>();

                    if (state.project.type == ProjectType::TwoDimensional)
                    {
                        const float min = glm::round(glm::min(entityTransform.scale.x, entityTransform.scale.y));
                        const float max = glm::round(glm::max(entityTransform.scale.x, entityTransform.scale.y));
                        float zoom = 1.f - min / max;
                        if (zoom == 0.f)
                            zoom = 1.f;

                        Camera2D& camera = state.editorScene.editorCamera2D;
                        camera.target = entityTransform.position;
                        camera.zoom = zoom;
                    }

                    if (state.project.type == ProjectType::ThreeDimensional)
                    {
                        EditorCamera3D& camera = state.editorScene.editorCamera3D;
                        camera.target = entityTransform.position;
                        camera.yaw = 39.5f;
                        camera.pitch = 31.3f;
                        camera.distance = 12.f;
                    }
                }
            }

            Scenes::UpdateEditor(*state.activeScene);
        }
        else
        {
            Input::Capture(SceneViewportPanel::IsFocused());
            Scenes::UpdateRuntime(*state.activeScene);
        }
    }

    void OnRender()
    {
        const Project& project = ProjectManager::GetActive();
        Entity activeCameraEntity2D = Scenes::GetActiveCameraEntity2D();
        Entity activeCameraEntity3D = Scenes::GetActiveCameraEntity3D();
        glm::vec3 clearColor = glm::vec3(0.f);
        if (activeCameraEntity2D != Entity_Null && project.type == ProjectType::TwoDimensional)
        {
            const auto& activeCameraComponent = activeCameraEntity2D.GetComponent<Camera2DComponent>();
            clearColor = activeCameraComponent.clearColor;
        }
        else if (activeCameraEntity3D != Entity_Null && project.type == ProjectType::ThreeDimensional)
        {
            const auto& activeCameraComponent = activeCameraEntity3D.GetComponent<Camera3DComponent>();
            clearColor = activeCameraComponent.clearColor;
        }
        else
            clearColor = glm::vec3(0.15f);

        Renderer::SetClearColor(V3_OPEN(clearColor));

        Framebuffers::Bind(state.framebuffer);
        RenderAPI::Clear();

        Framebuffers::ClearAttachment(state.framebuffer, 1, -1);

        if (state.sceneState == SceneState::Editor)
        {
            const bool isSceneViewportHovered = SceneViewportPanel::IsHovered();
            const bool isSceneViewportFocused = SceneViewportPanel::IsFocused();
            const glm::vec2 sceneViewportPosition = SceneViewportPanel::GetPosition();
            const glm::vec2 sceneViewportSize = SceneViewportPanel::GetSize();

            if (project.type == ProjectType::TwoDimensional)
            {
                Renderer::BeginScene2D(state.activeScene->editorCamera2D);
                Scenes::RenderEditor(*state.activeScene, SceneHeirarchyPanel::GetSelectedEntity());
                Renderer::EndScene2D();
            }
            else
            {
                Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();
                Renderer::BeginScene3D(state.activeScene->editorCamera3D);
                Scenes::RenderEditor(*state.activeScene, selectedEntity);
                Renderer::EndScene3D(selectedEntity);
            }

            Input::Capture(isSceneViewportHovered);
            if (Input::IsMouseClicked(MOUSE_BUTTON_LEFT) && isSceneViewportHovered && isSceneViewportFocused && !ImGuizmo::IsOver())
            {
                const glm::vec2 glViewportMouse = Utils::ScreenToViewportGL(Input::GetMousePosition(),
                                                                            sceneViewportPosition,
                                                                            sceneViewportSize);

                if (!Input::IsKeyDown(KEY_LEFT_ALT))
                {
                    state.pixelData = Framebuffers::ReadPixel(state.framebuffer, 1, (u32)glViewportMouse.x, (u32)glViewportMouse.y);

                    Scene* activeScene = CharmApp::GetActiveScene();
                    if (state.pixelData != -1)
                    {
                        Entity entity = Entities::Create((entt::entity)state.pixelData, activeScene);
                        SceneHeirarchyPanel::SetSelectedEntity(entity);
                    }
                    else
                        SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                }
                else if (Input::IsKeyDown(KEY_LEFT_ALT) && Input::IsKeyDown(KEY_LEFT_SHIFT) && state.project.type == ProjectType::TwoDimensional)
                {
                    Entity& entity = SceneHeirarchyPanel::GetSelectedEntity();
                    if (entity.IsHandleValid())
                    {
                        const glm::vec4 pixelColor = Framebuffers::ReadPixelColor(state.framebuffer, 0, (u32)glViewportMouse.x, (u32)glViewportMouse.y);
                        if (entity.HasComponent<SpriteRendererComponent>())
                        {
                            auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();
                            spriteRenderer.tint = pixelColor;
                        }

                        if (entity.HasComponent<Camera2DComponent>())
                        {
                            auto& cameraComponent = entity.GetComponent<Camera2DComponent>();
                            cameraComponent.clearColor = pixelColor;
                        }
                    }
                }
            }
        }
        else
            Scenes::RenderRuntime(*state.activeScene);

        if (state.project.grid.isEnabled)
        {
            const u32 colorAttachmentWidth = Framebuffers::GetColorAttachmentWidth(state.framebuffer);
            const u32 colorAttachmentHeight = Framebuffers::GetColorAttachmentHeight(state.framebuffer);
            const glm::vec2 resolution = glm::vec2(colorAttachmentWidth, colorAttachmentHeight);

            Renderer::DrawGrid(state.editorScene.editorCamera2D, resolution, state.project.grid.tileScale);
        }

        Framebuffers::Unbind();
    }

    void OnRenderUI()
    {
        ImGui::DockSpaceOverViewport();

        DrawMenuBar();
        DrawPreferencesMenu();

        ImGui::ShowDemoWindow();
        AnimationPanel::Display();
        AssetRegistryPanel::Display();
        ContentBrowserPanel::Display();
        DebugStatsPanel::Display();
        SceneHeirarchyPanel::Display();
        InspectorPanel::Display();
        TilePalettePanel::Display();
        ToolbarPanel::Display();
        SceneViewportPanel::Display(state.framebuffer);
        TextureSlicerPanel::Display();
    }

    void OnScenePlay()
    {
        Entity prev = SceneHeirarchyPanel::GetSelectedEntity();

        state.sceneState = SceneState::Runtime;
        state.runtimeScene = Scenes::Copy(state.editorScene);
        state.activeScene = &state.runtimeScene;
        state.project.isRuntime = true;

        SceneHeirarchyPanel::SetContext(*state.activeScene);
        Scenes::AlignParentsAndChildren(*state.activeScene);
        Scenes::OnRuntimeStart(*state.activeScene);

        if (prev.IsHandleValid())
        {
            auto& prevInternal = prev.GetComponent<InternalComponent>();
            SceneHeirarchyPanel::SetSelectedEntity(Entities::FindWithUUID(prevInternal.id, state.activeScene));
        }

        state.project.grid.isEnabled = false;
    }

    void OnSceneStop()
    {
        Entity prev = SceneHeirarchyPanel::GetSelectedEntity();

        SceneHeirarchyPanel::SetContext(state.editorScene);
        state.sceneState = SceneState::Editor;
        state.activeScene = &state.editorScene;
        state.project.isRuntime = false;

        if (prev.IsHandleValid())
        {
            auto& prevInternal = prev.GetComponent<InternalComponent>();
            SceneHeirarchyPanel::SetSelectedEntity(Entities::FindWithUUID(prevInternal.id, state.activeScene));
        }

        Scenes::OnRuntimeStop(state.runtimeScene);
        state.runtimeScene = Scenes::Create();

        if (state.project.type == ProjectType::TwoDimensional)
            state.project.grid.isEnabled = true;
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

        FileDialogFilter filter;
        filter.name = "Scene";
        filter.specification = "charm";

        if (FileDialogs::Open(&filter, 1))
        {
            const std::filesystem::path path = FileDialogs::GetSelectedPath();
            CharmApp::OpenScene(path);
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
        ProjectManager::Save(state.project);
        INFO("Saved scene %s", state.currentScenePath.c_str());
    }

    void OnSceneSaveAs()
    {
        FileDialogFilter filter;
        filter.name = "Scene";
        filter.specification = "charm";
        if (FileDialogs::Save(&filter, 1))
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

    void OnProjectOpen()
    {
        state.project = ProjectManager::New();

        FileDialogFilter filter;
        filter.name = "Project";
        filter.specification = "chprj";

        if (FileDialogs::Open(&filter, 1))
        {
            const std::filesystem::path path = FileDialogs::GetSelectedPath();
            state.project = ProjectManager::Load(path);
            if (state.project.type == ProjectType::TwoDimensional)
            {
                RenderAPI::DisableDepthBuffer();
                RenderAPI::DisableStencilBuffer();
            }
            if (state.project.type == ProjectType::ThreeDimensional)
            {
                RenderAPI::EnableDepthBuffer();
                RenderAPI::SetDepthFunc(BufferFunc::Less);
                RenderAPI::EnableStencilBuffer();
                RenderAPI::SetStencilOperation(StencilOperation::Keep, StencilOperation::Replace, StencilOperation::Replace);
            }

            ScriptManager::UnloadModule();

            const std::filesystem::path scriptModulePath = ProjectManager::GetScriptModulePath(state.project);
            if (std::filesystem::exists(scriptModulePath))
                ScriptManager::LoadModule(scriptModulePath);

            const std::filesystem::path startScenePath = ProjectManager::GetStartScenePath(state.project);
            const std::filesystem::path assetsPath = ProjectManager::GetAssetPath(state.project);
            OpenScene(startScenePath);
            ContentBrowserPanel::SetCurrentDirectory(assetsPath);
        }
    }

    void OpenScene(const std::filesystem::path& path)
    {
        OnSceneNew();

        SceneSerializer::SetContext(&state.editorScene);
        SceneSerializer::Deserialize(path);
        state.currentScenePath = path;
        ScriptManager::ReloadModule();

        if (state.project.type == ProjectType::TwoDimensional && Entities::FindWithTag("Main Camera", state.activeScene) == Entity_Null)
        {
            const auto& config = Application::GetConfig();
            Entity mainCamera = Scenes::CreateEntity(state.editorScene, "Main Camera");
            auto& cameraComponent = mainCamera.AddComponent<Camera2DComponent>();
            cameraComponent.camera.offset.x = (float)config.virtualWidth / (float)Application::GetPixelsPerUnit() / 2.f;
            cameraComponent.camera.offset.y = (float)config.virtualHeight / (float)Application::GetPixelsPerUnit() / 2.f;
            cameraComponent.isPrimary = true;
        }

        if (state.project.type == ProjectType::ThreeDimensional)
        {
            if (Entities::FindWithTag("Main Camera", state.activeScene) == Entity_Null)
            {
                Entity entity = Scenes::CreateEntity(state.editorScene, "Main Camera");
                entity.AddComponent<Camera3DComponent>();
            }

            if (Entities::FindWithTag("Directional Light", state.activeScene) == Entity_Null)
            {
                Entity entity = Scenes::CreateEntity(state.editorScene, "Directional Light");
                entity.AddComponent<DirectionalLightComponent>();
            }
        }

        if (std::filesystem::exists(state.currentScenePath))
        {
            const std::filesystem::path homeDirectory = Utils::GetHomeDirectory();
            const std::filesystem::path absolutePath = homeDirectory / std::filesystem::relative(path, homeDirectory);
            INFO("Loaded scene %s", absolutePath.c_str());
        }
    }

    void DrawMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New Scene", "Ctrl+N")) OnSceneNew();
                if (ImGui::MenuItem("Open Scene", "Ctrl+O")) OnSceneOpen();
                if (ImGui::MenuItem("Save Scene", "Ctrl+S")) OnSceneSave();
                if (ImGui::MenuItem("Save Scene As", "Ctrl+Shift+S")) OnSceneSaveAs();
                ImGui::Separator();
                if (ImGui::MenuItem("Open Project", "Ctrl+Shift+O")) OnProjectOpen();
                if (ImGui::MenuItem("Exit")) Application::Quit();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                ImGui::MenuItem("Undo", "Ctrl+Z");
                ImGui::MenuItem("Redo", "Ctrl+Y");
                ImGui::Separator();
                ImGui::MenuItem("Cut", "Ctrl+X");
                ImGui::MenuItem("Copy", "Ctrl+C");
                ImGui::MenuItem("Paste", "Ctrl+V");
                ImGui::Separator();
                if (ImGui::MenuItem("Preferences", "Ctrl+Shift+P", state.showPreferencesWindow)) state.showPreferencesWindow = !state.showPreferencesWindow;
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Animation", NULL, AnimationPanel::ShouldDisplay())) AnimationPanel::Toggle();
                if (ImGui::MenuItem("Debug Stats", NULL, DebugStatsPanel::ShouldDisplay())) DebugStatsPanel::Toggle();
                if (ImGui::MenuItem("Inspector", NULL, InspectorPanel::ShouldDisplay())) InspectorPanel::Toggle();
                if (ImGui::MenuItem("Scene Heirarchy", NULL, SceneHeirarchyPanel::ShouldDisplay())) SceneHeirarchyPanel::Toggle();
                if (ImGui::MenuItem("Tile Palette", NULL, TilePalettePanel::ShouldDisplay())) TilePalettePanel::Toggle();

                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void DrawPreferencesMenu()
    {
        if (state.showPreferencesWindow)
        {
            ImGui::Begin("Preferences", &state.showPreferencesWindow);

            if (ImGui::TreeNode("General"))
            {
                const float columnWidth = 125.f;
                u32 pixelsPerUnit = Application::GetPixelsPerUnit();
                UI::DrawIntInputControl("Pixels Per Unit", (s32*)&pixelsPerUnit, 1, 0, columnWidth);
                Application::SetPixelsPerUnit(pixelsPerUnit);

                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Editor Grid"))
            {
                const float columnWidth = 110.f;
                UI::DrawBoolControl("Is Enabled?", &state.project.grid.isEnabled, columnWidth);
                UI::DrawIntInputControl("Tile Scale", (s32*)&state.project.grid.tileScale, 1, 0, columnWidth);
                ImGui::TreePop();
            }

            ImGui::End();
        }
    }

    s32 GetPixelData() { return state.pixelData; }
    Scene* GetActiveScene() { return state.activeScene; }
    SceneState GetActiveSceneState() { return state.sceneState; }
    Project& GetProject() { return state.project; }

    void SetPixelData(s32 data) { state.pixelData = data; }
}
