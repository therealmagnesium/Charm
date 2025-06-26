#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Shader.h"
#include "Graphics/Shapes.h"
#include "Graphics/Texture.h"

#include "ECS/Components.h"

#include <glm/glm.hpp>

using namespace Charm::ECS;

namespace Charm
{
    namespace Graphics
    {
        enum class BatchMode : u8
        {
            Quads = 0,
            Circles,
        };

        struct QuadVertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 texCoord;
            u32 texIndex = 0;
            s32 entityID = -1;
        };

        struct CircleVertex
        {
            glm::vec3 worldPosition;
            glm::vec3 localPosition;
            glm::vec3 color;
            float thickness;
            float fade;
            s32 entityID = -1;
        };

        struct RenderState
        {
            glm::vec3 clearColor;
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            Shader defaultShader;
            Shader circleShader;
        };

        namespace Renderer
        {
            void Initialize();
            void Shutdown();

            void BeginScene2D(const Camera2D& camera);
            void BeginScene2D(const Camera3D& camera);
            void EndScene2D();

            void BeginBatchQuad();
            void EndBatchQuad();

            void BeginBatchCircle();
            void EndBatchCircle();

            void Flush(BatchMode mode);

            void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color);
            void DrawRectangleRec(const Rectangle& rectangle, const glm::vec3& color);
            void DrawRectanglePro(const Rectangle& rectangle, const glm::vec2& origin, float rotation, const glm::vec3& color);

            void DrawTexture(Texture& texture, const glm::vec2& position, const glm::vec3& tint);
            void DrawTextureEx(Texture& texture, const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec3& tint);
            void DrawTextureRec(Texture& texture, Rectangle& source, const glm::vec2& position, const glm::vec3& tint);
            void DrawTexturePro(Texture& texture, Rectangle& source, Rectangle& dest, const glm::vec2& origin, float rotation, const glm::vec3& tint);

            void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color);
            void DrawCirclePro(const glm::vec2& center, float radius, float thickness, float fade, const glm::vec3& color);

            void DrawEntity(const glm::mat4& transform, SpriteRendererComponent& spriteRenderer, s32 entityID);
            void DrawEntity(const glm::mat4& transform, CircleRendererComponent& circleRenderer, s32 entityID);

            glm::vec3& GetClearColor();
            u32 GetQuadCount();
            u32 GetCircleCount();
            u32 GetDrawCount();

            void SetClearColor(float r, float g, float b);
        }
    }
}
