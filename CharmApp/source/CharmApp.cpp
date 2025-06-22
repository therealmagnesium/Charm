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

    void OnCreate()
    {
        const ApplicationConfig& config = Application::GetConfig();
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);

        FramebufferSpecification framebufferSpec;
        framebufferSpec.width = config.virtualWidth;
        framebufferSpec.height = config.virtualHeight;
        framebufferSpec.numAttachments = 2;
        state.framebuffer = Framebuffers::Create(framebufferSpec);

        state.textures[0] = AssetManager::Import("assets/textures/small_checker.png", AssetType::Texture);
        state.textures[1] = AssetManager::Import("assets/textures/texel_checker.png", AssetType::Texture);

        state.scene = Scenes::Create();
        SceneSerializer::SetContext(state.scene);
        SceneHeirarchyPanel::SetContext(state.scene);
    }

    void OnUpdate()
    {
        if (SceneViewportPanel::IsFocused())
            Input::Capture(true);
        else
            Input::Capture(false);

        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_S))
        {
            SceneSerializer::Serialize("assets/scenes/Untitled.charm");
            INFO("Saved scene to assets/scenes/Untitled.charm");
        }

        if (Input::IsKeyDown(KEY_LEFT_CTRL) && Input::IsKeyPressed(KEY_O))
        {
            SceneSerializer::Deserialize("assets/scenes/Untitled.charm");
            INFO("Loaded scene assets/scenes/Untitled.charm");
        }

        Scenes::Update(state.scene);
    }

    void OnRender()
    {
        Framebuffers::Bind(state.framebuffer);
        RenderCommand::Clear();

        Scenes::Render(state.scene);

        Framebuffers::Unbind();
    }

    void OnRenderUI()
    {
        const ApplicationConfig& config = Application::GetConfig();
        const glm::vec2 virtualMousePosition = Utils::ScreenToVirtual(Input::GetMousePosition());
        const glm::vec2 viewportMousePosition = Utils::ScreenToViewport(Input::GetMousePosition(), SceneViewportPanel::GetPosition(), SceneViewportPanel::GetSize());

        ImGui::DockSpaceOverViewport();

        SceneHeirarchyPanel::Display();
        SceneViewportPanel::Display(state.framebuffer);

        if (SceneViewportPanel::IsFocused())
        {
            RenderCommand::HideCursor();
            if (!SceneViewportPanel::IsHovered())
                RenderCommand::ShowCursor();
        }

        ImGui::Begin("Debug Stats");
        ImGui::Text("FPS: %d", (u32)(1.f / Time::GetDelta()));
        ImGui::Text("MS per frame: %.7f", Time::GetDelta());
        ImGui::Text("Number of quads: %d", Renderer::GetQuadCount());
        ImGui::Text("Number of circles: %d", Renderer::GetCircleCount());
        ImGui::Text("Number of draw calls: %d", Renderer::GetDrawCount());
        ImGui::Text("Virtual mouse position: " V2_FMT, V2_OPEN(virtualMousePosition));
        ImGui::Text("Viewport mouse position: " V2_FMT, V2_OPEN(viewportMousePosition));
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
    }

    void OnShutdown()
    {
        AssetManager::Clean();
        Framebuffers::Destroy(state.framebuffer);
    }
}
