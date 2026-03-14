#include "Graphics/Renderer.h"
#include "Graphics/RenderAPI.h"
#include "Graphics/RendererInternals.h"
#include "Graphics/Camera.h"
#include "Graphics/Material.h"
#include "Graphics/Mesh.h"
#include "Graphics/Model.h"
#include "Graphics/Shapes.h"
#include "Graphics/Window.h"

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <ImGuizmo.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

using namespace Charm::Core;
using namespace Charm::Projects;

namespace Charm::Graphics
{
    const static u32 k_MaxQuadCount = 5000;
    const static u32 k_MaxVertexCount = k_MaxQuadCount * 4;
    const static u32 k_MaxIndexCount = k_MaxQuadCount * 6;
    const static u32 k_MaxTextures = 32;
    const static u32 k_MaxInstances = 4096;

    struct BatchData
    {
        u32 quadVertexArray = 0;
        u32 quadVertexBuffer = 0;

        u32 circleVertexArray = 0;
        u32 circleVertexBuffer = 0;

        u32 lineVertexArray = 0;
        u32 lineVertexBuffer = 0;

        u32 indexBuffer = 0;

        Texture whiteTexture;

        u32 quadIndexCount = 0;
        QuadVertex* quadBuffer = NULL;
        QuadVertex* quadBufferRef = NULL;

        u32 circleIndexCount = 0;
        CircleVertex* circleBuffer = NULL;
        CircleVertex* circleBufferRef = NULL;

        u32 lineVertexCount = 0;
        LineVertex* lineBuffer = NULL;
        LineVertex* lineBufferRef = NULL;

        Texture textureSlots[k_MaxTextures];
        u32 textureSlotIndex = 1;

        glm::vec4 quadVertexPositions[4];
        glm::vec4 circleVertexPositions[4];

        u32 drawCount = 0;
        u32 quadCount = 0;
        u32 circleCount = 0;
        u32 lineCount = 0;
    };

    static RenderState state;
    static BatchData batchData;
    static bool isInitialized = false;

    namespace Renderer
    {
        void SetupBatchRendering();
        void CleanUpBatchRendering();
        void CheckForNewBatch(BatchMode mode);
        float CheckBatchForTextureIndex(Texture& texture);
        void AddQuadToBatch(const glm::mat4& transform, Rectangle& source, u32 textureIndex, const glm::vec2& textureSize, const glm::vec4& color);
        void AddCircleToBatch(const glm::mat4& transform, const glm::vec3& color, float thickness, float fade);
        void AddEntityToBatch(const glm::mat4& transform, const SpriteRendererComponent& spriteRenderer, s32 entityID);
        void AddEntityToBatch(const glm::mat4& transform, const CircleRendererComponent& circleRenderer, s32 entityID);
        void SubmitCommand(const RenderCommand& command);
        void DrawSelectionContextOutline(const Entity& selectionContext);
        bool WriteCommandToStencil(const RenderCommand& command, const Entity& selectionContext);

        void Initialize()
        {
            if (isInitialized)
            {
                WARN("Cannot initialize the renderer more than once");
                return;
            }

            ASSERT(SDL_Init(SDL_INIT_VIDEO) != false, "Failed to initialize SDL3!");

            const ApplicationConfig& config = Application::GetConfig();
            Window::Initialize(config.virtualWidth, config.virtualHeight, config.name.c_str());

            gladLoadGL();
            INFO("OpenGL functions have successfully loaded");
            INFO("GPU vendor: %s", glGetString(GL_VENDOR));
            INFO("GPU specs: %s", glGetString(GL_RENDERER));
            INFO("OpenGL version: %s", glGetString(GL_VERSION));

            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            const Project& project = ProjectManager::GetActive();
            if (project.type == ProjectType::ThreeDimensional)
            {
                RenderAPI::EnableDepthBuffer();
                RenderAPI::SetDepthFunc(BufferFunc::Less);
                RenderAPI::EnableStencilBuffer();
                RenderAPI::SetStencilOperation(StencilOperation::Keep, StencilOperation::Replace, StencilOperation::Replace);
            }

            state.viewMatrix = glm::mat4(1.f);
            state.projectionMatrix = glm::mat4(1.f);
            state.commands.reserve(8);

            state.quadShader = Shaders::Load("assets/shaders/BatchingQuads_vs.glsl", "assets/shaders/BatchingQuads_fs.glsl");
            Shaders::CreateUniform(state.quadShader, "viewMatrix");
            Shaders::CreateUniform(state.quadShader, "projectionMatrix");
            Shaders::CreateUniform(state.quadShader, "textures");

            state.circleShader = Shaders::Load("assets/shaders/BatchingCircles_vs.glsl", "assets/shaders/BatchingCircles_fs.glsl");
            Shaders::CreateUniform(state.circleShader, "viewMatrix");
            Shaders::CreateUniform(state.circleShader, "projectionMatrix");

            state.lineShader = Shaders::Load("assets/shaders/BatchingLines_vs.glsl", "assets/shaders/BatchingLines_fs.glsl");
            Shaders::CreateUniform(state.lineShader, "viewMatrix");
            Shaders::CreateUniform(state.lineShader, "projectionMatrix");

            state.diffuseShader = Shaders::Load("assets/shaders/Diffuse_vs.glsl", "assets/shaders/Diffuse_fs.glsl");
            Shaders::CreateUniform(state.diffuseShader, "u_entityID");
            Shaders::CreateUniform(state.diffuseShader, "u_matrixTransform");
            Shaders::CreateUniform(state.diffuseShader, "u_matrixView");
            Shaders::CreateUniform(state.diffuseShader, "u_matrixProjection");
            Shaders::CreateUniform(state.diffuseShader, "u_material.albedo");
            Shaders::CreateUniform(state.diffuseShader, "u_material.albedoTexture");

            state.blinnPhongShader = Shaders::Load("assets/shaders/Blinn-Phong_vs.glsl", "assets/shaders/Blinn-Phong_fs.glsl");
            Shaders::CreateUniform(state.blinnPhongShader, "u_cameraPosition");
            Shaders::CreateUniform(state.blinnPhongShader, "u_matrixView");
            Shaders::CreateUniform(state.blinnPhongShader, "u_matrixProjection");
            Shaders::CreateUniform(state.blinnPhongShader, "u_material.albedo");
            Shaders::CreateUniform(state.blinnPhongShader, "u_material.albedoTexture");
            Shaders::CreateUniform(state.blinnPhongShader, "u_sun.direction");
            Shaders::CreateUniform(state.blinnPhongShader, "u_sun.color");
            Shaders::CreateUniform(state.blinnPhongShader, "u_sun.intensity");

            state.outlineShader = Shaders::Load("assets/shaders/Outline_vs.glsl", "assets/shaders/Outline_fs.glsl");
            Shaders::CreateUniform(state.outlineShader, "u_matrixView");
            Shaders::CreateUniform(state.outlineShader, "u_matrixProjection");

            state.grid.vao = VertexArray::Create();
            state.grid.shader = Shaders::Load("assets/shaders/2DGrid_vs.glsl", "assets/shaders/2DGrid_fs.glsl");
            Shaders::CreateUniform(state.grid.shader, "u_cameraPosition");
            Shaders::CreateUniform(state.grid.shader, "u_cameraZoom");
            Shaders::CreateUniform(state.grid.shader, "u_resolution");
            Shaders::CreateUniform(state.grid.shader, "u_pixelsPerUnit");
            Shaders::CreateUniform(state.grid.shader, "u_tileScale");

            state.clearColor = glm::vec3(0.91f);

            SetupBatchRendering();

            INFO("The renderer was successfully initialized");
            isInitialized = true;
        }

        void Shutdown()
        {
            INFO("The renderer is shutting down...");

            CleanUpBatchRendering();

            Shaders::Unload(state.quadShader);
            Shaders::Unload(state.circleShader);
            Shaders::Unload(state.lineShader);
            Shaders::Unload(state.diffuseShader);
            Shaders::Unload(state.blinnPhongShader);
            Shaders::Unload(state.outlineShader);

            Shaders::Unload(state.grid.shader);
            VertexArray::Destroy(state.grid.vao);

            Window::Shutdown();
            SDL_Quit();

            isInitialized = false;
        }

        void BeginScene2D(const Camera2D& camera)
        {
            const ApplicationConfig& config = Application::GetConfig();
            state.viewMatrix = Cameras::GetViewMatrix2D(camera);
            state.projectionMatrix = Cameras::GetProjectionMatrix2D(camera);
            batchData.quadCount = 0;
            batchData.circleCount = 0;
            batchData.lineCount = 0;
            batchData.drawCount = 0;

            Shaders::Bind(state.quadShader);
            Shaders::SetUniform(state.quadShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.quadShader, "projectionMatrix", state.projectionMatrix);

            Shaders::Bind(state.circleShader);
            Shaders::SetUniform(state.circleShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.circleShader, "projectionMatrix", state.projectionMatrix);

            Shaders::Bind(state.lineShader);
            Shaders::SetUniform(state.lineShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.lineShader, "projectionMatrix", state.projectionMatrix);

            BeginBatchQuad();
            BeginBatchCircle();
            BeginBatchLine();
        }

        void BeginScene2D(const EditorCamera3D& camera)
        {
            state.viewMatrix = Cameras::GetViewMatrix3D(camera);
            state.projectionMatrix = Cameras::GetProjectionMatrix3D(camera);
            batchData.quadCount = 0;
            batchData.circleCount = 0;
            batchData.lineCount = 0;
            batchData.drawCount = 0;

            Shaders::Bind(state.quadShader);
            Shaders::SetUniform(state.quadShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.quadShader, "projectionMatrix", state.projectionMatrix);

            Shaders::Bind(state.circleShader);
            Shaders::SetUniform(state.circleShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.circleShader, "projectionMatrix", state.projectionMatrix);

            Shaders::Bind(state.lineShader);
            Shaders::SetUniform(state.lineShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.lineShader, "projectionMatrix", state.projectionMatrix);

            BeginBatchQuad();
            BeginBatchCircle();
            BeginBatchLine();
        }

        void BeginScene3D(const EditorCamera3D& camera)
        {
            BeginScene2D(camera);

            state.viewMatrix = Cameras::GetViewMatrix3D(camera);
            state.projectionMatrix = Cameras::GetProjectionMatrix3D(camera);

            Shaders::Bind(state.diffuseShader);
            Shaders::SetUniform(state.diffuseShader, "u_matrixView", state.viewMatrix);
            Shaders::SetUniform(state.diffuseShader, "u_matrixProjection", state.projectionMatrix);

            const glm::vec3 cameraPosition = glm::vec3(glm::inverse(state.viewMatrix)[3]);
            Shaders::Bind(state.blinnPhongShader);
            Shaders::SetUniform(state.blinnPhongShader, "u_matrixView", state.viewMatrix);
            Shaders::SetUniform(state.blinnPhongShader, "u_matrixProjection", state.projectionMatrix);
            Shaders::SetUniform(state.blinnPhongShader, "u_cameraPosition", cameraPosition);

            Shaders::Bind(state.outlineShader);
            Shaders::SetUniform(state.outlineShader, "u_matrixView", state.viewMatrix);
            Shaders::SetUniform(state.outlineShader, "u_matrixProjection", state.projectionMatrix);
        }

        void BeginScene2D(const SceneCamera3D& camera)
        {
            state.viewMatrix = Cameras::GetViewMatrix3D(camera);
            state.projectionMatrix = Cameras::GetProjectionMatrix3D(camera);
            batchData.quadCount = 0;
            batchData.circleCount = 0;
            batchData.lineCount = 0;
            batchData.drawCount = 0;

            Shaders::Bind(state.quadShader);
            Shaders::SetUniform(state.quadShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.quadShader, "projectionMatrix", state.projectionMatrix);

            Shaders::Bind(state.circleShader);
            Shaders::SetUniform(state.circleShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.circleShader, "projectionMatrix", state.projectionMatrix);

            Shaders::Bind(state.lineShader);
            Shaders::SetUniform(state.lineShader, "viewMatrix", state.viewMatrix);
            Shaders::SetUniform(state.lineShader, "projectionMatrix", state.projectionMatrix);

            BeginBatchQuad();
            BeginBatchCircle();
            BeginBatchLine();
        }

        void BeginScene3D(const SceneCamera3D& camera)
        {
            BeginScene2D(camera);

            state.viewMatrix = Cameras::GetViewMatrix3D(camera);
            state.projectionMatrix = Cameras::GetProjectionMatrix3D(camera);

            Shaders::Bind(state.diffuseShader);
            Shaders::SetUniform(state.diffuseShader, "u_matrixView", state.viewMatrix);
            Shaders::SetUniform(state.diffuseShader, "u_matrixProjection", state.projectionMatrix);

            Shaders::Bind(state.blinnPhongShader);
            Shaders::SetUniform(state.blinnPhongShader, "u_matrixView", state.viewMatrix);
            Shaders::SetUniform(state.blinnPhongShader, "u_matrixProjection", state.projectionMatrix);
        }

        void EndScene2D()
        {
            EndBatchQuad();
            EndBatchCircle();
            EndBatchLine();

            Flush(BatchMode::Quads);
            Flush(BatchMode::Circles);
            Flush(BatchMode::Lines);
        }

        void EndScene3D(Entity& selectionContext)
        {
            const u32 commandCount = state.commands.size();
            std::vector<RenderCommand> commandsOpaque;
            std::vector<RenderCommand> commandsTranslucent;

            commandsOpaque.reserve(commandCount);
            commandsTranslucent.reserve(commandCount);

            for (const RenderCommand& command : state.commands)
            {
                if (command.material == NULL || command.mesh == NULL)
                    return;

                if (command.material->shader == NULL || command.mesh->indices.size() < 1)
                    return;

                if (!command.material->IsTranslucent())
                    commandsOpaque.emplace_back(command);
                else
                    commandsTranslucent.emplace_back(command);
            }

            const glm::vec3 cameraPosition = glm::vec3(glm::inverse(state.viewMatrix)[3]);
            const auto SortByDistance = [&](const RenderCommand& a, const RenderCommand& b)
            {
                const float distanceA = glm::distance2(cameraPosition, a.worldCenter);
                const float distanceB = glm::distance2(cameraPosition, b.worldCenter);
                return distanceA > distanceB;
            };
            std::sort(commandsTranslucent.begin(), commandsTranslucent.end(), SortByDistance);

            for (const RenderCommand& command : commandsOpaque)
            {
                if (!WriteCommandToStencil(command, selectionContext))
                    SubmitCommand(command);
            }

            for (const RenderCommand& command : commandsTranslucent)
            {
                if (!WriteCommandToStencil(command, selectionContext))
                    SubmitCommand(command);
            }

            if (selectionContext.IsHandleValid())
                DrawSelectionContextOutline(selectionContext);

            state.commands.clear();
            state.commands.reserve(commandCount);

            EndScene2D();
        }

        void BeginBatchQuad()
        {
            batchData.quadIndexCount = 0;
            batchData.quadBufferRef = batchData.quadBuffer;

            batchData.textureSlotIndex = 1;
        }

        void EndBatchQuad()
        {
            u64 size = (u8*)batchData.quadBufferRef - (u8*)batchData.quadBuffer;

            glBindBuffer(GL_ARRAY_BUFFER, batchData.quadVertexBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, batchData.quadBuffer);
        }

        void BeginBatchCircle()
        {
            batchData.circleIndexCount = 0;
            batchData.circleBufferRef = batchData.circleBuffer;
        }

        void EndBatchCircle()
        {
            u64 size = (u8*)batchData.circleBufferRef - (u8*)batchData.circleBuffer;

            glBindBuffer(GL_ARRAY_BUFFER, batchData.circleVertexBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, batchData.circleBuffer);
        }

        void BeginBatchLine()
        {
            batchData.lineVertexCount = 0;
            batchData.lineBufferRef = batchData.lineBuffer;
        }

        void EndBatchLine()
        {
            u64 size = (u8*)batchData.lineBufferRef - (u8*)batchData.lineBuffer;

            glBindBuffer(GL_ARRAY_BUFFER, batchData.lineVertexBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size, batchData.lineBuffer);
        }

        void Flush(BatchMode mode)
        {
            if (batchData.quadIndexCount > 0 && mode == BatchMode::Quads)
            {
                for (u32 i = 0; i < batchData.textureSlotIndex; i++)
                    Textures::Bind(batchData.textureSlots[i], i);

                Shaders::Bind(state.quadShader);
                glBindVertexArray(batchData.quadVertexArray);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchData.indexBuffer);
                glDrawElements(GL_TRIANGLES, batchData.quadIndexCount, GL_UNSIGNED_INT, NULL);

                batchData.drawCount++;
            }

            if (batchData.circleIndexCount > 0 && mode == BatchMode::Circles)
            {
                Shaders::Bind(state.circleShader);
                glBindVertexArray(batchData.circleVertexArray);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchData.indexBuffer);
                glDrawElements(GL_TRIANGLES, batchData.circleIndexCount, GL_UNSIGNED_INT, NULL);

                batchData.drawCount++;
            }

            if (batchData.lineVertexCount > 0 && mode == BatchMode::Lines)
            {
                Shaders::Bind(state.lineShader);
                glBindVertexArray(batchData.lineVertexArray);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glDrawArrays(GL_LINES, 0, batchData.lineVertexCount);

                batchData.drawCount++;
            }
        }

        void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
        {
            Rectangle rectangle;
            rectangle.x = position.x;
            rectangle.y = position.y;
            rectangle.width = size.x;
            rectangle.height = size.y;

            DrawRectanglePro(rectangle, glm::vec2(0.f), 0.f, color);
        }

        void DrawRectangleRec(const Rectangle& rectangle, const glm::vec4& color)
        {
            DrawRectanglePro(rectangle, glm::vec2(0.f), 0.f, color);
        }

        void DrawRectanglePro(const Rectangle& rectangle, const glm::vec2& origin, float rotation, const glm::vec4& color)
        {
            CheckForNewBatch(BatchMode::Quads);

            const glm::vec3 position = glm::vec3(rectangle.x, rectangle.y, 0.f);
            const glm::vec2 size = glm::vec2(rectangle.width, rectangle.height);
            const glm::mat4 transform = Utils::GetTransformMatrix2D(position, size, rotation, origin);
            const u32 textureIndex = 0;

            Rectangle source;
            source.width = 1.f;
            source.height = 1.f;

            AddQuadToBatch(transform, source, textureIndex, glm::vec2(1.f), color);
        }

        void DrawTexture(Texture& texture, const glm::vec2& position, const glm::vec4& tint)
        {
            Rectangle source;
            source.width = texture.width;
            source.height = texture.height;

            Rectangle dest;
            dest.x = position.x;
            dest.y = position.y;
            dest.width = texture.width;
            dest.height = texture.height;

            DrawTexturePro(texture, source, dest, glm::vec2(0.f), 0.f, tint);
        }

        void DrawTextureEx(Texture& texture, const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec4& tint)
        {
            Rectangle source;
            source.width = texture.width;
            source.height = texture.height;

            Rectangle dest;
            dest.x = position.x;
            dest.y = position.y;

            if (texture.id != 0)
            {
                dest.width = texture.width * scale.x;
                dest.height = texture.height * scale.y;
            }
            else
            {
                dest.width = texture.width * scale.x;
                dest.height = texture.height * scale.y;
            }

            glm::vec2 origin;
            origin.x = texture.width * scale.x / 2.f;
            origin.y = texture.height * scale.y / 2.f;

            DrawTexturePro(texture, source, dest, origin, rotation, tint);
        }

        void DrawTextureRec(Texture& texture, Rectangle& source, const glm::vec2& position, const glm::vec4& tint)
        {
            Rectangle dest;
            dest.x = position.x;
            dest.y = position.y;
            dest.width = texture.width;
            dest.height = texture.height;

            DrawTexturePro(texture, source, dest, glm::vec2(0.f), 0.f, tint);
        }

        void DrawTexturePro(Texture& texture, Rectangle& source, Rectangle& dest, const glm::vec2& origin, float rotation, const glm::vec4& tint)
        {
            CheckForNewBatch(BatchMode::Quads);

            const glm::vec3 position = glm::vec3(dest.x, dest.y, 0.f);
            const glm::vec2 size = glm::vec2(dest.width, dest.height);
            const glm::vec2 textureSize = glm::vec2(texture.width, texture.height);
            const u32 textureIndex = CheckBatchForTextureIndex(texture);
            const glm::mat4 transform = Utils::GetTransformMatrix2D(position, size, rotation, origin);

            AddQuadToBatch(transform, source, textureIndex, textureSize, tint);
        }

        void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color)
        {
            DrawCirclePro(center, radius, 1.f, 0.05f, color);
        }

        void DrawCirclePro(const glm::vec2& center, float radius, float thickness, float fade, const glm::vec3& color)
        {
            CheckForNewBatch(BatchMode::Circles);

            const glm::vec2 size = glm::vec2(radius);
            const glm::mat4 transform = Utils::GetTransformMatrix2D(glm::vec3(center, 0.f), size, 0.f, glm::vec2(0.f));

            AddCircleToBatch(transform, color, thickness, fade);
        }

        void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& color)
        {
            DrawLineEx(p0, p1, 4.f, color);
        }

        void DrawLineEx(const glm::vec3& p0, const glm::vec3& p1, float lineWidth, const glm::vec3& color)
        {
            CheckForNewBatch(BatchMode::Lines);
            RenderAPI::SetLineWidth(lineWidth);

            batchData.lineBufferRef->position = p0;
            batchData.lineBufferRef->color = color;
            batchData.lineBufferRef++;

            batchData.lineBufferRef->position = p1;
            batchData.lineBufferRef->color = color;
            batchData.lineBufferRef++;

            batchData.lineVertexCount += 2;
            batchData.lineCount++;
        }

        void DrawRectangleLines(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color)
        {
            glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, 0.f);
            glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, 0.f);
            glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, 0.f);
            glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, 0.f);

            DrawLine(p0, p1, color);
            DrawLine(p1, p2, color);
            DrawLine(p2, p3, color);
            DrawLine(p3, p0, color);
        }

        void DrawRectangleLines(const glm::mat4& transform, const glm::vec3& color)
        {
            glm::vec3 lineVertices[4];
            for (u8 i = 0; i < LEN(lineVertices); i++)
                lineVertices[i] = transform * batchData.quadVertexPositions[i];

            DrawLineEx(lineVertices[0], lineVertices[1], 6.f, color);
            DrawLineEx(lineVertices[1], lineVertices[2], 6.f, color);
            DrawLineEx(lineVertices[2], lineVertices[3], 6.f, color);
            DrawLineEx(lineVertices[3], lineVertices[0], 6.f, color);
        }

        void DrawMesh(Mesh& mesh, const Material& material, const glm::mat4& transform, s32 entityID)
        {
            if (mesh.instanceBuffer == 0)
                Meshes::SetupInstanceBuffer(mesh, k_MaxInstances);

            RenderCommand command;
            command.mesh = &mesh;
            command.material = &material;
            command.transform = transform;
            command.worldCenter = glm::vec3(transform * glm::vec4(mesh.GetCenter(), 1.f));
            command.entityID = entityID;
            command.instanceCount = 1;

            state.commands.emplace_back(command);
        }

        void DrawMeshInstanced(Mesh& mesh, const Material& material, const InstanceData* data, u32 count)
        {
            if (count == 0)
                return;

            if (material.IsTranslucent())
            {
                for (u32 i = 0; i < count; i++)
                    DrawMesh(mesh, material, data[i].transform, data[i].entityID);
                return;
            }

            if (mesh.instanceBuffer == 0)
                Meshes::SetupInstanceBuffer(mesh, k_MaxInstances);

            RenderCommand command;
            command.mesh = &mesh;
            command.material = &material;
            command.worldCenter = glm::vec3(data[0].transform * glm::vec4(mesh.GetCenter(), 1.f));
            command.entityID = data[0].entityID;
            command.instanceCount = count;
            command.instanceData.assign(data, data + count);

            state.commands.emplace_back(std::move(command));
        }

        void DrawModel(Model& model, const glm::mat4& transform, Shader& shader, s32 entityID)
        {
            for (u32 i = 0; i < model.meshes.size(); i++)
            {
                Mesh& mesh = model.meshes[i];
                Material& material = model.materials[mesh.materialIndex];

                if (shader != Shader_Invalid)
                    material.shader = &shader;

                DrawMesh(mesh, material, transform, entityID);
            }
        }

        void DrawEntity(const glm::mat4& transform, const SpriteRendererComponent& spriteRenderer, s32 entityID)
        {
            CheckForNewBatch(BatchMode::Quads);
            AddEntityToBatch(transform, spriteRenderer, entityID);
        }

        void DrawEntity(const glm::mat4& transform, const CircleRendererComponent& circleRenderer, s32 entityID)
        {
            CheckForNewBatch(BatchMode::Circles);
            AddEntityToBatch(transform, circleRenderer, entityID);
        }

        void DrawGrid(const Camera2D& camera, const glm::vec2& resolution, u32 tileScale)
        {
            const u32 quadVertexCount = 6;

            VertexArray::Bind(state.grid.vao);
            Shaders::Bind(state.grid.shader);

            Shaders::SetUniform(state.grid.shader, "u_cameraPosition", camera.target);
            Shaders::SetUniform(state.grid.shader, "u_cameraZoom", camera.zoom);
            Shaders::SetUniform(state.grid.shader, "u_resolution", resolution);
            Shaders::SetUniform(state.grid.shader, "u_pixelsPerUnit", Application::GetPixelsPerUnit());
            Shaders::SetUniform(state.grid.shader, "u_tileScale", tileScale);
            RenderAPI::DrawArrays(PrimitiveType::Triangles, quadVertexCount);

            Shaders::Unbind();
            VertexArray::Unbind();
        }

        Shader& GetShaderDiffuse() { return state.diffuseShader; }
        Shader& GetShaderBlinnPhong() { return state.blinnPhongShader; }
        Shader& GetShaderOutline() { return state.outlineShader; }
        glm::vec3& GetClearColor() { return state.clearColor; }
        const glm::mat4& GetViewMatrix() { return state.viewMatrix; }
        const glm::mat4& GetProjectionMatrix() { return state.projectionMatrix; }
        u32 GetQuadCount() { return batchData.quadCount; }
        u32 GetCircleCount() { return batchData.circleCount; }
        u32 GetLineCount() { return batchData.lineCount; }
        u32 GetDrawCount() { return batchData.drawCount; }

        void SetClearColor(float r, float g, float b)
        {
            state.clearColor.r = r;
            state.clearColor.g = g;
            state.clearColor.b = b;
        }

        void SetupBatchRendering()
        {
            // Rectangles / quads
            batchData.quadVertexArray = VertexArray::Create();
            batchData.quadVertexBuffer = VertexBuffer::Create();
            batchData.indexBuffer = IndexBuffer::Create();

            VertexArray::Bind(batchData.quadVertexArray);

            VertexBuffer::Bind(batchData.quadVertexBuffer);
            VertexBuffer::SetData(k_MaxVertexCount * sizeof(QuadVertex), NULL, GL_DYNAMIC_DRAW);

            VertexArray::EnableAttributeLocation(0);
            VertexArray::SpecifyFormat(0, 3, GL_FLOAT, sizeof(QuadVertex), offsetof(QuadVertex, position));

            VertexArray::EnableAttributeLocation(1);
            VertexArray::SpecifyFormat(1, 4, GL_FLOAT, sizeof(QuadVertex), offsetof(QuadVertex, color));

            VertexArray::EnableAttributeLocation(2);
            VertexArray::SpecifyFormat(2, 2, GL_FLOAT, sizeof(QuadVertex), offsetof(QuadVertex, texCoord));

            VertexArray::EnableAttributeLocation(3);
            VertexArray::SpecifyFormat(3, 2, GL_FLOAT, sizeof(QuadVertex), offsetof(QuadVertex, tilingFactor));

            VertexArray::EnableAttributeLocation(4);
            VertexArray::SpecifyFormat(4, 1, GL_UNSIGNED_INT, sizeof(QuadVertex), offsetof(QuadVertex, texIndex));

            VertexArray::EnableAttributeLocation(5);
            VertexArray::SpecifyFormat(5, 1, GL_INT, sizeof(QuadVertex), offsetof(QuadVertex, entityID));

            batchData.quadBuffer = new QuadVertex[k_MaxVertexCount];

            // Circles
            batchData.circleVertexArray = VertexArray::Create();
            batchData.circleVertexBuffer = VertexBuffer::Create();

            VertexArray::Bind(batchData.circleVertexArray);

            VertexBuffer::Bind(batchData.circleVertexBuffer);
            VertexBuffer::SetData(k_MaxVertexCount * sizeof(CircleVertex), NULL, GL_DYNAMIC_DRAW);

            VertexArray::EnableAttributeLocation(0);
            VertexArray::SpecifyFormat(0, 3, GL_FLOAT, sizeof(CircleVertex), offsetof(CircleVertex, worldPosition));

            VertexArray::EnableAttributeLocation(1);
            VertexArray::SpecifyFormat(1, 3, GL_FLOAT, sizeof(CircleVertex), offsetof(CircleVertex, localPosition));

            VertexArray::EnableAttributeLocation(2);
            VertexArray::SpecifyFormat(2, 3, GL_FLOAT, sizeof(CircleVertex), offsetof(CircleVertex, color));

            VertexArray::EnableAttributeLocation(3);
            VertexArray::SpecifyFormat(3, 1, GL_FLOAT, sizeof(CircleVertex), offsetof(CircleVertex, thickness));

            VertexArray::EnableAttributeLocation(4);
            VertexArray::SpecifyFormat(4, 1, GL_FLOAT, sizeof(CircleVertex), offsetof(CircleVertex, fade));

            VertexArray::EnableAttributeLocation(5);
            VertexArray::SpecifyFormat(5, 1, GL_INT, sizeof(CircleVertex), offsetof(CircleVertex, entityID));

            batchData.circleBuffer = new CircleVertex[k_MaxVertexCount];

            // Lines
            batchData.lineVertexArray = VertexArray::Create();
            batchData.lineVertexBuffer = VertexBuffer::Create();

            VertexArray::Bind(batchData.lineVertexArray);

            VertexBuffer::Bind(batchData.lineVertexBuffer);
            VertexBuffer::SetData(k_MaxVertexCount * sizeof(LineVertex), NULL, GL_DYNAMIC_DRAW);

            VertexArray::EnableAttributeLocation(0);
            VertexArray::SpecifyFormat(0, 3, GL_FLOAT, sizeof(LineVertex), offsetof(LineVertex, position));

            VertexArray::EnableAttributeLocation(1);
            VertexArray::SpecifyFormat(1, 3, GL_FLOAT, sizeof(LineVertex), offsetof(LineVertex, color));

            batchData.lineBuffer = new LineVertex[k_MaxVertexCount];

            // All
            s32 samplers[k_MaxTextures];
            for (u32 i = 0; i < k_MaxTextures; i++)
                samplers[i] = i;

            Shaders::Bind(state.quadShader);
            Shaders::SetUniform(state.quadShader, "textures", samplers, k_MaxTextures);

            batchData.whiteTexture = Textures::LoadDefaultWhite();
            batchData.textureSlots[0] = batchData.whiteTexture;

            batchData.quadVertexPositions[0] = glm::vec4(0.f, 1.f, 0.f, 1.f);
            batchData.quadVertexPositions[1] = glm::vec4(1.f, 1.f, 0.f, 1.f);
            batchData.quadVertexPositions[2] = glm::vec4(1.f, 0.f, 0.f, 1.f);
            batchData.quadVertexPositions[3] = glm::vec4(0.f, 0.f, 0.f, 1.f);

            batchData.circleVertexPositions[0] = glm::vec4(-0.5f, -0.5f, 0.f, 1.f);
            batchData.circleVertexPositions[1] = glm::vec4(0.5f, -0.5f, 0.f, 1.f);
            batchData.circleVertexPositions[2] = glm::vec4(0.5f, 0.5f, 0.f, 1.f);
            batchData.circleVertexPositions[3] = glm::vec4(-0.5f, 0.5f, 0.f, 1.f);

            u32* indices = new u32[k_MaxIndexCount];
            u32 offset = 0;

            for (u32 i = 0; i < k_MaxIndexCount; i += 6)
            {
                indices[i + 0] = 0 + offset;
                indices[i + 1] = 1 + offset;
                indices[i + 2] = 2 + offset;

                indices[i + 3] = 2 + offset;
                indices[i + 4] = 3 + offset;
                indices[i + 5] = 0 + offset;

                offset += 4;
            }

            IndexBuffer::Bind(batchData.indexBuffer);
            IndexBuffer::SetData(k_MaxIndexCount * sizeof(u32), indices, GL_STATIC_DRAW);

            delete[] indices;
        }

        void CleanUpBatchRendering()
        {
            delete[] batchData.quadBuffer;
            delete[] batchData.circleBuffer;
            delete[] batchData.lineBuffer;

            VertexArray::Destroy(batchData.quadVertexArray);
            VertexBuffer::Destroy(batchData.quadVertexBuffer);

            VertexArray::Destroy(batchData.circleVertexArray);
            VertexBuffer::Destroy(batchData.circleVertexBuffer);

            glDeleteVertexArrays(1, &batchData.lineVertexArray);
            glDeleteBuffers(1, &batchData.lineVertexBuffer);

            IndexBuffer::Destroy(batchData.indexBuffer);
            Textures::Unload(batchData.whiteTexture);
        }

        void CheckForNewBatch(BatchMode mode)
        {
            if (mode == BatchMode::Quads && batchData.quadIndexCount >= k_MaxIndexCount)
            {
                EndBatchQuad();
                Flush(mode);
                BeginBatchQuad();
            }

            if (mode == BatchMode::Circles && batchData.circleIndexCount >= k_MaxIndexCount)
            {
                EndBatchCircle();
                Flush(mode);
                BeginBatchCircle();
            }

            if (mode == BatchMode::Lines && batchData.lineVertexCount >= k_MaxVertexCount)
            {
                EndBatchLine();
                Flush(mode);
                BeginBatchLine();
            }
        }

        float CheckBatchForTextureIndex(Texture& texture)
        {
            float textureIndex = 0.f;

            if (texture.id != 0)
            {
                for (u32 i = 1; i < batchData.textureSlotIndex; i++)
                {
                    if (batchData.textureSlots[i] == texture)
                    {
                        textureIndex = (float)i;
                        break;
                    }
                }

                if (textureIndex == 0.f)
                {
                    textureIndex = (float)batchData.textureSlotIndex;
                    batchData.textureSlots[batchData.textureSlotIndex] = texture;
                    batchData.textureSlotIndex++;
                }
            }

            return textureIndex;
        }

        void AddQuadToBatch(const glm::mat4& transform, Rectangle& source, u32 textureIndex, const glm::vec2& textureSize, const glm::vec4& color)
        {
            glm::vec2 textureCoords[4];
            textureCoords[0] = glm::vec2(source.x / textureSize.x, source.y / textureSize.y);
            textureCoords[1] = glm::vec2((source.x + source.width) / textureSize.x, source.y / textureSize.y);
            textureCoords[2] = glm::vec2((source.x + source.width) / textureSize.x, (source.y + source.height) / textureSize.y);
            textureCoords[3] = glm::vec2(source.x / textureSize.x, (source.y + source.height) / textureSize.y);

            for (u8 i = 0; i < 4; i++)
            {
                batchData.quadBufferRef->position = transform * batchData.quadVertexPositions[i];
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = textureCoords[i];
                batchData.quadBufferRef->tilingFactor = glm::vec2(1.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef->entityID = -1;
                batchData.quadBufferRef++;
            }

            batchData.quadIndexCount += 6;
            batchData.quadCount++;
        }

        void AddCircleToBatch(const glm::mat4& transform, const glm::vec3& color, float thickness, float fade)
        {
            for (u8 i = 0; i < 4; i++)
            {
                batchData.circleBufferRef->worldPosition = transform * batchData.circleVertexPositions[i];
                batchData.circleBufferRef->localPosition = batchData.circleVertexPositions[i] * 2.f;
                batchData.circleBufferRef->color = color;
                batchData.circleBufferRef->thickness = thickness;
                batchData.circleBufferRef->fade = fade;
                batchData.circleBufferRef->entityID = -1;
                batchData.circleBufferRef++;
            }

            batchData.circleIndexCount += 6;
            batchData.circleCount++;
        }

        void AddEntityToBatch(const glm::mat4& transform, const SpriteRendererComponent& spriteRenderer, s32 entityID)
        {
            Texture* texture = AssetManager::GetAsset<Texture>(spriteRenderer.sprite);
            glm::vec2 textureSize = glm::vec2(1.f);
            u32 textureIndex = 0;

            if (texture != NULL)
            {
                textureSize = glm::vec2(texture->width, texture->height);
                textureIndex = CheckBatchForTextureIndex(*texture);
            }

            glm::vec2 textureCoords[4];
            textureCoords[0] = glm::vec2(spriteRenderer.crop.x / textureSize.x, spriteRenderer.crop.y / textureSize.y);
            textureCoords[1] = glm::vec2((spriteRenderer.crop.x + spriteRenderer.crop.width) / textureSize.x, spriteRenderer.crop.y / textureSize.y);
            textureCoords[2] = glm::vec2((spriteRenderer.crop.x + spriteRenderer.crop.width) / textureSize.x, (spriteRenderer.crop.y + spriteRenderer.crop.height) / textureSize.y);
            textureCoords[3] = glm::vec2(spriteRenderer.crop.x / textureSize.x, (spriteRenderer.crop.y + spriteRenderer.crop.height) / textureSize.y);

            for (u8 i = 0; i < 4; i++)
            {
                batchData.quadBufferRef->position = transform * batchData.quadVertexPositions[i];
                batchData.quadBufferRef->color = spriteRenderer.tint;
                batchData.quadBufferRef->texCoord = textureCoords[i];
                batchData.quadBufferRef->tilingFactor = spriteRenderer.tilingFactor;
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef->entityID = entityID;
                batchData.quadBufferRef++;
            }

            batchData.quadIndexCount += 6;
            batchData.quadCount++;
        }

        void AddEntityToBatch(const glm::mat4& transform, const CircleRendererComponent& circleRenderer, s32 entityID)
        {
            for (u8 i = 0; i < 4; i++)
            {
                batchData.circleBufferRef->worldPosition = transform * batchData.circleVertexPositions[i];
                batchData.circleBufferRef->localPosition = batchData.circleVertexPositions[i] * 2.f;
                batchData.circleBufferRef->color = circleRenderer.color;
                batchData.circleBufferRef->thickness = circleRenderer.thickness;
                batchData.circleBufferRef->fade = circleRenderer.fade;
                batchData.circleBufferRef->entityID = entityID;
                batchData.circleBufferRef++;
            }

            batchData.circleIndexCount += 6;
            batchData.circleCount++;
        }

        void SubmitCommand(const RenderCommand& command)
        {
            // For single draws, wrap the transform + entityID into a temporary InstanceData.
            // For instanced draws, the instanceData vector already contains per-instance values.
            InstanceData singleData;
            const InstanceData* data = nullptr;
            u32 instanceCount = 0;

            if (command.instanceData.empty())
            {
                singleData.transform = command.transform;
                singleData.entityID = command.entityID;
                data = &singleData;
                instanceCount = 1;
            }
            else
            {
                data = command.instanceData.data();
                instanceCount = command.instanceCount;
            }

            Meshes::UploadInstanceData(*command.mesh, data, instanceCount);

            VertexArray::Bind(command.mesh->vertexArray);
            IndexBuffer::Bind(command.mesh->indexBuffer);

            Shaders::Bind(*command.material->shader);
            Shaders::SetUniform(*command.material->shader, "u_material.albedo", command.material->albedo);
            Shaders::SetUniform(*command.material->shader, "u_material.albedoTexture", 0);

            const Texture& albedoTexture = command.material->albedoTexture != NULL ? *command.material->albedoTexture : batchData.whiteTexture;
            Textures::Bind(albedoTexture, 0);

            RenderAPI::DrawIndexedInstanced(PrimitiveType::Triangles, command.mesh->indices.size(), instanceCount);

            Shaders::Unbind();
            IndexBuffer::Unbind();
            VertexArray::Unbind();
        }

        void DrawSelectionContextOutline(const Entity& selectionContext)
        {
            const auto& internal = selectionContext.GetComponent<InternalComponent>();
            if (!internal.isActive || !selectionContext.HasComponent<MeshRendererComponent>())
                return;

            auto& transform = selectionContext.GetComponent<TransformComponent>();
            const auto& meshRenderer = selectionContext.GetComponent<MeshRendererComponent>();
            const float scale = 1.1f;

            transform.scale *= scale;
            if (AssetManager::IsHandleValid(meshRenderer.model))
            {
                const glm::mat4 transformMatrix = transform.GetMatrix3D();
                Shader& outlineShader = Renderer::GetShaderOutline();
                Model* model = AssetManager::GetAsset<Model>(meshRenderer.model);

                RenderAPI::SetStencilFunc(BufferFunc::NotEqual, 1, 0xFF);
                RenderAPI::DisableStencilWriting();
                RenderAPI::DisableDepthBuffer();

                for (const Mesh& mesh : model->meshes)
                {
                    Material outlineMaterial = model->materials[mesh.materialIndex]; // Copy so the model asset is never mutated
                    outlineMaterial.shader = &outlineShader;

                    RenderCommand command;
                    command.mesh = &mesh;
                    command.material = &outlineMaterial;
                    command.transform = transformMatrix;
                    command.entityID = (s32)selectionContext.handle;
                    command.instanceCount = 1;
                    SubmitCommand(command);
                }

                RenderAPI::EnableStencilWriting();
                RenderAPI::EnableDepthBuffer();
            }
            transform.scale /= scale;
        }

        // Sets stencil state and submits the command. For single-instance commands the
        // caller is responsible for calling SubmitCommand (returns false). For multi-instance
        // commands the batch is split into a selected sub-command and a non-selected
        // sub-command which are both submitted here (returns true so the caller skips its
        // own SubmitCommand call).
        bool WriteCommandToStencil(const RenderCommand& command, const Entity& selectionContext)
        {
            if (command.instanceCount == 1)
            {
                const entt::entity handle = (entt::entity)command.entityID;
                const Entity entity = Entities::Create(handle, selectionContext.context);

                if (entity == selectionContext && selectionContext.IsHandleValid())
                {
                    RenderAPI::SetStencilFunc(BufferFunc::Always, 1, 0xFF);
                    RenderAPI::EnableStencilWriting();
                }
                else
                {
                    RenderAPI::SetStencilFunc(BufferFunc::Always, 0, 0xFF);
                    RenderAPI::DisableStencilWriting();
                }

                // Caller submits.
                return false;
            }

            // Multi-instance path: split instanceData into two sub-commands so each
            // group gets its own draw call under the correct stencil state.
            // This is necessary because a single glDrawElementsInstanced call uses one
            // stencil state for all instances — per-instance stencil toggling is not
            // possible without splitting the draw.
            RenderCommand selectedCommand = command;
            RenderCommand otherCommand = command;
            selectedCommand.instanceData.clear();
            otherCommand.instanceData.clear();

            for (const InstanceData& instance : command.instanceData)
            {
                const entt::entity handle = (entt::entity)instance.entityID;
                const Entity entity = Entities::Create(handle, selectionContext.context);

                if (entity == selectionContext && selectionContext.IsHandleValid())
                    selectedCommand.instanceData.push_back(instance);
                else
                    otherCommand.instanceData.push_back(instance);
            }

            if (!otherCommand.instanceData.empty())
            {
                otherCommand.instanceCount = (u32)otherCommand.instanceData.size();
                RenderAPI::SetStencilFunc(BufferFunc::Always, 0, 0xFF);
                RenderAPI::DisableStencilWriting();
                SubmitCommand(otherCommand);
            }

            if (!selectedCommand.instanceData.empty())
            {
                selectedCommand.instanceCount = (u32)selectedCommand.instanceData.size();
                RenderAPI::SetStencilFunc(BufferFunc::Always, 1, 0xFF);
                RenderAPI::EnableStencilWriting();
                SubmitCommand(selectedCommand);
            }

            // Both sub-commands submitted here; caller must not call SubmitCommand again.
            return true;
        }
    }
}
