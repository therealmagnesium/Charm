#include "CharmApp.h"

#include <Charm.h>
#include <imgui.h>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmApp
{
    static CharmState state;

    void OnCreate()
    {
        const ApplicationConfig& config = Application::GetConfig();
        Renderer::SetClearColor(0.15f, 0.15f, 0.17f);

        state.camera.target = glm::vec2(0.f, 0.f);
        // state.camera.offset.x = config.virtualWidth / 2.f;
        // state.camera.offset.y = config.virtualHeight / 2.f;

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
        glm::vec2 playerDirection;
        playerDirection.x = Input::GetInputAxisAlt(InputAxis::Horizontal);
        playerDirection.y = Input::GetInputAxisAlt(InputAxis::Vertical);
        if (playerDirection.x != 0.f && playerDirection.y != 0.f)
            playerDirection = glm::normalize(playerDirection);
        state.playerPosition.x += playerDirection.x * playerSpeed * Time::GetDelta();
        state.playerPosition.y += playerDirection.y * playerSpeed * Time::GetDelta();
    }

    void OnRender()
    {
        const ApplicationConfig& config = Application::GetConfig();
        const u32 tileSize = 8;

        Renderer::BeginScene2D(state.camera);

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

        Renderer::DrawRectangle(state.playerPosition.x - 32.f, state.playerPosition.y - 32.f, 64.f, 64.f, glm::vec3(1.f));
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
            ImGui::End();
        }
    }

    void OnShutdown()
    {
    }
}
