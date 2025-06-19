#include "CharmApp.h"
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

    void DrawBackground(float tileSize, float spacing, float offset);

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

        state.playerPosition.x = config.virtualWidth / 2.f;
        state.playerPosition.y = config.virtualHeight / 2.f;

        state.scene = Scenes::Create();

        state.entity = Scenes::CreateEntity(state.scene);
        state.entity.AddComponent<SpriteRendererComponent>(state.textures[0]);

        state.circle = Scenes::CreateEntity(state.scene, "Circle");
        state.circle.AddComponent<CircleRendererComponent>(0.7f, 1.f, 0.5f, glm::vec3(0.8f, 0.72f, 0.2f));
    }

    void OnUpdate()
    {
        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        Scenes::Update(state.scene);

        const float playerSpeed = 650.f;

        state.playerDirection.x = Input::GetInputAxisAlt(InputAxis::Horizontal);
        state.playerDirection.y = Input::GetInputAxisAlt(InputAxis::Vertical);

        if (state.playerDirection.x != 0.f && state.playerDirection.y != 0.f)
            state.playerDirection = glm::normalize(state.playerDirection);

        state.playerPosition.x += state.playerDirection.x * playerSpeed * Time::GetDelta();
        state.playerPosition.y += state.playerDirection.y * playerSpeed * Time::GetDelta();

        auto& entityTransform = state.entity.GetComponent<TransformComponent>();
        entityTransform.scale.x = 0.25f;
        entityTransform.scale.y = 0.25f;
        entityTransform.position = glm::vec3(state.playerPosition, 0.f);

        if (SceneViewportPanel::IsFocused())
        {
            const glm::vec2 virtualMousePosition = Utils::ScreenToVirtual(Input::GetMousePosition());
            const glm::vec2 viewportMousePosition = Utils::ScreenToViewport(Input::GetMousePosition(), SceneViewportPanel::GetPosition(), SceneViewportPanel::GetSize());
            auto& circleTransform = state.circle.GetComponent<TransformComponent>();
            circleTransform.position = glm::vec3(viewportMousePosition, 0.f);
        }

        state.circle.isActive = SceneViewportPanel::IsFocused();
    }

    void OnRender()
    {
        Framebuffers::Bind(state.framebuffer);
        RenderCommand::Clear();

        Renderer::BeginScene2D(state.camera);

        Scenes::Render(state.scene);

        Renderer::EndScene2D();
        Framebuffers::Unbind();
    }

    void OnRenderUI()
    {
        const ApplicationConfig& config = Application::GetConfig();
        const glm::vec2 virtualMousePosition = Utils::ScreenToVirtual(Input::GetMousePosition());
        const glm::vec2 viewportMousePosition = Utils::ScreenToViewport(Input::GetMousePosition(), SceneViewportPanel::GetPosition(), SceneViewportPanel::GetSize());

        ImGui::DockSpaceOverViewport();

        SceneViewportPanel::Display(state.framebuffer);

        if (SceneViewportPanel::IsFocused())
            RenderCommand::HideCursor();

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

        ImGui::Begin("Controls");
        ImGui::DragFloat("Tile size", &state.tileSize, 1.f, 4.f, 128.f);
        ImGui::DragFloat("Tile spacing", &state.tileSpacing, 1.f, 0.f, 32.f);
        ImGui::DragFloat("Tile offset", &state.tileOffset, 1.f);
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

    void DrawBackground(float tileSize, float spacing, float offset)
    {
        const ApplicationConfig& config = Application::GetConfig();

        glm::vec3 color = glm::vec3(1.f);
        Rectangle rectangle;

        for (s32 i = (s32)tileSize / 2; i <= config.virtualHeight + tileSize; i += tileSize + spacing)
        {
            for (s32 j = (s32)tileSize / 2; j <= config.virtualWidth; j += tileSize + spacing)
            {
                rectangle.x = j + offset;
                rectangle.y = i + offset;
                rectangle.width = tileSize;
                rectangle.height = tileSize;

                color.r = 0.5f;
                color.g = ((float)j / 8.f) / 255.f;
                color.b = ((float)i / 8.f) / 255.f;

                Renderer::DrawRectanglePro(rectangle, glm::vec2(tileSize / 2.f), 0.f, color);
            }
        }
    }
}
