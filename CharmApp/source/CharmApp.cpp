#include "CharmApp.h"

#include <Charm.h>
#include <imgui.h>
#include <glad/glad.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static CharmState state;

    void DrawBackground();

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

        if (Input::IsKeyPressed(KEY_0))
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

        DrawBackground();
        Renderer::DrawTexture(state.texture, state.playerPosition, glm::vec2(64.f), glm::vec3(1.f));

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
        }
    }

    void OnShutdown()
    {
        Textures::Unload(state.texture);
    }

    void DrawBackground()
    {
        const ApplicationConfig& config = Application::GetConfig();
        const u32 tileSize = 16;

        glm::vec3 color;
        for (s32 i = 0; i < config.virtualHeight; i += tileSize)
        {
            for (s32 j = 0; j < config.virtualWidth; j += tileSize)
            {
                color.r = 1.f;
                color.g = (j / (float)tileSize) / 255.f;
                color.b = (i / (float)tileSize) / 255.f;
                Renderer::DrawRectangle(j, i, tileSize, tileSize, color);
            }
        }
    }
}
