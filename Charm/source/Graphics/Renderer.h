#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
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
            Lines,
        };

        struct QuadVertex
        {
            glm::vec3 position;
            glm::vec4 color;
            glm::vec2 texCoord;
            glm::vec2 tilingFactor = glm::vec2(1.f);
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

        struct LineVertex
        {
            glm::vec3 position;
            glm::vec3 color;
        };

        struct RendererGrid
        {
            u32 vao = 0;
            Shader shader;
        };

        struct RenderCommand
        {
            std::vector<InstanceData> instanceData;
            glm::mat4 transform = glm::mat4(1.f);
            glm::vec3 worldCenter = glm::vec3(0.f);
            const Mesh* mesh = NULL;
            const Material* material = NULL;
            u32 instanceCount = 0;
            s32 entityID = -1;
        };

        struct RenderState
        {
            glm::vec3 clearColor;
            glm::mat4 viewMatrix;
            glm::mat4 projectionMatrix;
            Shader quadShader;
            Shader circleShader;
            Shader lineShader;
            Shader diffuseShader;
            Shader blinnPhongShader;
            Shader outlineShader;
            RendererGrid grid;
            std::vector<RenderCommand> commands;
        };

        namespace Renderer
        {
            void Initialize();
            void Shutdown();

            void BeginScene2D(const Camera2D& camera);
            void BeginScene2D(const EditorCamera3D& camera);
            void BeginScene3D(const EditorCamera3D& camera);
            void BeginScene2D(const SceneCamera3D& camera);
            void BeginScene3D(const SceneCamera3D& camera);
            void EndScene2D();
            void EndScene3D(Entity& selectionContext = (Entity&)Entity_Null);

            void BeginBatchQuad();
            void EndBatchQuad();

            void BeginBatchCircle();
            void EndBatchCircle();

            void BeginBatchLine();
            void EndBatchLine();

            void Flush(BatchMode mode);

            Shader& GetShaderDiffuse();
            Shader& GetShaderBlinnPhong();
            Shader& GetShaderOutline();
            glm::vec3& GetClearColor();
            const glm::mat4& GetViewMatrix();
            const glm::mat4& GetProjectionMatrix();
            u32 GetQuadCount();
            u32 GetCircleCount();
            u32 GetLineCount();
            u32 GetDrawCount();

            void SetClearColor(float r, float g, float b);

            void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
            void DrawRectangleRec(const Rectangle& rectangle, const glm::vec4& color);
            void DrawRectanglePro(const Rectangle& rectangle, const glm::vec2& origin, float rotation, const glm::vec4& color);

            void DrawTexture(Texture& texture, const glm::vec2& position, const glm::vec4& tint);
            void DrawTextureEx(Texture& texture, const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec4& tint);
            void DrawTextureRec(Texture& texture, Rectangle& source, const glm::vec2& position, const glm::vec4& tint);
            void DrawTexturePro(Texture& texture, Rectangle& source, Rectangle& dest, const glm::vec2& origin, float rotation, const glm::vec4& tint);

            void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color);
            void DrawCirclePro(const glm::vec2& center, float radius, float thickness, float fade, const glm::vec3& color);

            void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& color);
            void DrawLineEx(const glm::vec3& p0, const glm::vec3& p1, float lineWidth, const glm::vec3& color);
            void DrawRectangleLines(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color);
            void DrawRectangleLines(const glm::mat4& transform, const glm::vec3& color);

            void DrawMesh(Mesh& mesh, const Material& material, const glm::mat4& transform, s32 entityID = -1);
            void DrawMeshInstanced(Mesh& mesh, const Material& material, const InstanceData* data, u32 count);
            void DrawModel(Model& model, const glm::mat4& transform, Shader& shader = (Shader&)Shader_Invalid, s32 entityID = -1);

            void DrawEntity(const glm::mat4& transform, const SpriteRendererComponent& spriteRenderer, s32 entityID);
            void DrawEntity(const glm::mat4& transform, const CircleRendererComponent& circleRenderer, s32 entityID);

            void DrawGrid(const Camera2D& camera, const glm::vec2& resolution, u32 tileScale = 1);
        }
    }
}
