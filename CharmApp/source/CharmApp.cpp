#include "CharmApp.h"
#include "Panels/SceneHeirarchyPanel.h"
#include "Panels/SceneViewport.h"

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

    void OnScenePlay();
    void OnSceneStop();

    void OnCreate()
    {
        const ApplicationConfig& config = Application::GetConfig();
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);

        FramebufferSpecification framebufferSpec;
        framebufferSpec.width = config.virtualWidth;
        framebufferSpec.height = config.virtualHeight;
        framebufferSpec.attachments = {TextureFormat::RGBA, TextureFormat::RedInteger, TextureFormat::DepthStencil};
        state.framebuffer = Framebuffers::Create(framebufferSpec);

        state.iconPlay = Textures::Load("assets/textures/charm/play_button.png");
        state.iconStop = Textures::Load("assets/textures/charm/stop_button.png");

        AssetManager::Import("assets/textures/small_checker.png", AssetType::Texture);
        AssetManager::Import("assets/textures/texel_checker.png", AssetType::Texture);

        state.scene = Scenes::Create();
        SceneSerializer::SetContext(state.scene);
        SceneHeirarchyPanel::SetContext(state.scene);
    }

    void OnUpdate()
    {
        Input::Capture(true);

        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        if (Input::IsKeyPressed(KEY_F2))
            Scenes::ResetEditorCameras(state.scene);

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_S))
        {
            if (FileDialogs::Save())
            {
                const std::string& path = FileDialogs::GetSelectedPath();
                SceneSerializer::Serialize(path.c_str());
                INFO("Saved scene to %s", path.c_str());
            }
        }

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_O))
        {
            if (FileDialogs::Open())
            {
                const std::string& path = FileDialogs::GetSelectedPath();
                SceneSerializer::Deserialize(path.c_str());
                INFO("Loaded scene %s", path.c_str());
            }
        }

        Input::Capture(SceneViewportPanel::IsFocused());
        Application::SetViewportPosition(SceneViewportPanel::GetPosition());
        Application::SetViewportSize(SceneViewportPanel::GetSize());

        if (state.scene.state == SceneState::Editor)
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

            Scenes::UpdateEditor(state.scene);
        }
        else
            Scenes::UpdateRuntime(state.scene);
    }

    void OnRender()
    {
        Framebuffers::Bind(state.framebuffer);
        RenderCommand::Clear();
        Framebuffers::ClearAttachment(state.framebuffer, 1, -1);

        if (state.scene.state == SceneState::Editor)
        {
            Scenes::RenderEditor(state.scene);
        }
        else
            Scenes::RenderRuntime(state.scene);

        Framebuffers::Unbind();
    }

    void OnRenderUI()
    {
        const ApplicationConfig& config = Application::GetConfig();
        const glm::vec2 virtualMouse = Utils::ScreenToVirtual(Input::GetMousePosition());

        glm::vec2 viewportMouse = Utils::ScreenToViewport(Input::GetMousePosition(),
                                                          SceneViewportPanel::GetPosition(),
                                                          SceneViewportPanel::GetSize());

        glm::vec2 glViewportMouse = Utils::ScreenToViewportGL(Input::GetMousePosition(),
                                                              SceneViewportPanel::GetPosition(),
                                                              SceneViewportPanel::GetSize());

        ImGui::DockSpaceOverViewport();

        SceneHeirarchyPanel::Display();
        SceneViewportPanel::Display(state.framebuffer.colorAttachments[0]);

        ImGui::Begin("Debug Stats");
        ImGui::Text("FPS: %d", (u32)(1.f / Time::GetDelta()));
        ImGui::Text("MS per frame: %.7f", Time::GetDelta());
        ImGui::Text("Number of quads: %d", Renderer::GetQuadCount());
        ImGui::Text("Number of circles: %d", Renderer::GetCircleCount());
        ImGui::Text("Number of draw calls: %d", Renderer::GetDrawCount());
        ImGui::Text("Editor camera distance: %.2f", state.scene.editorCamera3D.distance);
        ImGui::Text("Pixel data: %d", state.pixelData);
        ImGui::Text("Virtual mouse position: " V2_FMT, V2_OPEN(virtualMouse));
        ImGui::Text("Viewport mouse position: " V2_FMT, V2_OPEN(viewportMouse));
        ImGui::Text("GL mouse position: " V2_FMT, V2_OPEN(glViewportMouse));
        ImGui::Text("Is viewport hovered?: %s", Utils::BoolToCString(SceneViewportPanel::IsHovered()));
        ImGui::Text("Is viewport focused?: %s", Utils::BoolToCString(SceneViewportPanel::IsFocused()));
        ImGui::End();

        ImGui::Begin("Asset Registry");

        for (auto& [handle, metadata] : AssetManager::GetRegistry())
        {
            ImGui::Text("Handle: 0x%lx", handle);
            ImGui::Text("Path: %s", metadata.path.c_str());
            ImGui::Text("Type: %s", Utils::AssetTypeToString(metadata.type).c_str());
        }

        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 2.f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.f, 0.f));

        const auto& colors = ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(V3_OPEN(buttonHovered), 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(V3_OPEN(buttonActive), 0.5f));
        ImGui::Begin("##Toolbar", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        float iconSize = ImGui::GetWindowHeight() - 12.f;
        ImTextureID icon = (state.scene.state == SceneState::Editor) ? state.iconPlay.id : state.iconStop.id;
        ImGui::SetCursorPosX((ImGui::GetContentRegionMax().x * 0.5f) - (iconSize * 0.5f));
        if (ImGui::ImageButton("##PlayButton", icon, ImVec2(iconSize, iconSize), ImVec2(0.f, 1.f), ImVec2(1.f, 0.f)))
        {
            if (state.scene.state == SceneState::Editor)
                OnScenePlay();
            else
                OnSceneStop();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
        ImGui::End();
    }

    void OnShutdown()
    {
        Textures::Unload(state.iconPlay);
        Textures::Unload(state.iconStop);

        AssetManager::Clean();
        Framebuffers::Destroy(state.framebuffer);
    }

    void OnScenePlay()
    {
        state.scene.state = SceneState::Runtime;
    }

    void OnSceneStop()
    {
        state.scene.state = SceneState::Editor;
    }
}
