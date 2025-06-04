#include "CharmApp.h"

#include <Charm.h>
#include <imgui.h>
#include <glad/glad.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static CharmState state;

    void DrawBackground(float tileSize, float spacing, float offset);

    void OnCreate()
    {
        const ApplicationConfig& config = Application::GetConfig();
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);

        state.texture = Textures::Load("assets/textures/small_checker.png");

        state.playerPosition.x = config.virtualWidth / 2.f;
        state.playerPosition.y = config.virtualHeight / 2.f;
    }

    void OnUpdate()
    {
        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        if (Input::IsKeyPressed(KEY_F2))
            state.showDebugUI = !state.showDebugUI;

        const float playerSpeed = 650.f;

        state.playerDirection.x = Input::GetInputAxisAlt(InputAxis::Horizontal);
        state.playerDirection.y = Input::GetInputAxisAlt(InputAxis::Vertical);

        if (state.playerDirection.x != 0.f && state.playerDirection.y != 0.f)
            state.playerDirection = glm::normalize(state.playerDirection);

        state.playerPosition.x += state.playerDirection.x * playerSpeed * Time::GetDelta();
        state.playerPosition.y += state.playerDirection.y * playerSpeed * Time::GetDelta();
    }

    void OnRender()
    {
        Renderer::BeginScene2D(state.camera);

        DrawBackground(state.tileSize, state.tileSpacing, state.tileOffset);
        Renderer::DrawTexturePro(state.texture, state.playerPosition, glm::vec2(64.f), 0.f, glm::vec2(32.f), glm::vec3(1.f));

        Renderer::EndScene2D();
    }

    void OnRenderUI()
    {
        if (state.showDebugUI)
        {
            ImGui::Begin("Debug Stats");
            ImGui::Text("FPS: %d", (u32)(1.f / Time::GetDelta()));
            ImGui::Text("MS per frame: %.3f", Time::GetDelta());
            ImGui::Text("Player position: " V2_FMT, V2_OPEN(state.playerPosition));
            ImGui::Text("Number of quads: %d", Renderer::GetQuadCount());
            ImGui::Text("Number of draw calls: %d", Renderer::GetDrawCount());
            ImGui::End();

            ImGui::Begin("Controls");
            ImGui::DragFloat("Tile size", &state.tileSize, 1.f, 4.f, 128.f);
            ImGui::DragFloat("Tile spacing", &state.tileSpacing, 1.f, 0.f, 32.f);
            ImGui::DragFloat("Tile offset", &state.tileOffset, 1.f);
            ImGui::End();
        }
    }

    void OnShutdown()
    {
        Textures::Unload(state.texture);
    }

    void DrawBackground(float tileSize, float spacing, float offset)
    {
        const ApplicationConfig& config = Application::GetConfig();

        glm::vec3 color;
        for (s32 i = (s32)tileSize / 2; i <= config.virtualHeight; i += tileSize + spacing)
        {
            for (s32 j = (s32)tileSize / 2; j <= config.virtualWidth; j += tileSize + spacing)
            {
                color.r = 0.5f;
                color.g = ((float)j / 8.f) / 255.f;
                color.b = ((float)i / 8.f) / 255.f;
                Renderer::DrawRectanglePro(glm::vec2(j + offset, i + offset), glm::vec2(tileSize), 0.f, glm::vec2(tileSize / 2.f), color);
            }
        }
    }
}
