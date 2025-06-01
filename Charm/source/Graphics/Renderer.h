#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Shader.h"

#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct Vertex
        {
            glm::vec3 position;
        };

        struct RenderState
        {
            glm::vec3 clearColor;
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            Shader defaultShader;
        };

        namespace Renderer
        {
            void Initialize();
            void Shutdown();

            void BeginScene2D(const Camera2D& camera);
            void EndScene2D();

            void BeginBatch();
            void EndBatch();
            void Flush();

            void DrawRectangle(float x, float y, float width, float height, const glm::vec3& color);

            glm::vec3& GetClearColor();
            void SetClearColor(float r, float g, float b);
        }
    }
}
