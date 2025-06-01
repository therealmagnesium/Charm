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
        state.camera.offset.x = config.virtualWidth / 2.f;
        state.camera.offset.y = config.virtualHeight / 2.f;

        const float size = 128.f;
        float vertices[] = {
            0.5f * size, 0.5f * size, 0.0f,   // top right
            0.5f * size, -0.5f * size, 0.0f,  // bottom right
            -0.5f * size, -0.5f * size, 0.0f, // bottom left
            -0.5f * size, 0.5f * size, 0.0f   // top left
        };

        u32 indices[] = {
            0, 1, 3, // first Triangle
            1, 2, 3  // second Triangle
        };

        glGenVertexArrays(1, &state.vertexArray);
        glGenBuffers(1, &state.vertexBuffer);
        glGenBuffers(1, &state.indexBuffer);

        glBindVertexArray(state.vertexArray);

        glBindBuffer(GL_ARRAY_BUFFER, state.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, 3 * sizeof(float), NULL);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void OnUpdate()
    {
        if (Input::IsKeyPressed(KEY_ESCAPE))
            Application::Quit();

        if (Input::IsKeyPressed(KEY_0))
            state.showDebugUI = !state.showDebugUI;
    }

    void OnRender()
    {
        Renderer::BeginScene2D(state.camera);

        glBindVertexArray(state.vertexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state.indexBuffer);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        Renderer::EndScene2D();
    }

    void OnRenderUI()
    {
        if (state.showDebugUI)
            ImGui::ShowDemoWindow();
    }

    void OnShutdown()
    {
        glDeleteVertexArrays(1, &state.vertexArray);
        glDeleteBuffers(1, &state.vertexBuffer);
        glDeleteBuffers(1, &state.indexBuffer);
    }
}
