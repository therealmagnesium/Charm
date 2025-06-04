#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
#include "Graphics/Shader.h"

#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 texCoord;
            float texIndex;
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

            void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color);
            void DrawRectanglePro(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin, const glm::vec3& color);
            void DrawTexture(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec3& tint);
            void DrawTexturePro(Texture& texture, const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec2& origin, const glm::vec3& tint);

            glm::vec3& GetClearColor();
            u32 GetQuadCount();
            u32 GetDrawCount();

            void SetClearColor(float r, float g, float b);
        }
    }
}
