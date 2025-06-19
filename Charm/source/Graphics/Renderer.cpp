#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <SDL3/SDL.h>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        const static u32 k_MaxQuadCount = 5000;
        const static u32 k_MaxVertexCount = k_MaxQuadCount * 4;
        const static u32 k_MaxIndexCount = k_MaxQuadCount * 6;
        const static u32 k_MaxTextures = 32;

        struct BatchData
        {
            u32 quadVertexArray = 0;
            u32 quadVertexBuffer = 0;

            u32 indexBuffer = 0;

            u32 circleVertexArray = 0;
            u32 circleVertexBuffer = 0;

            Texture whiteTexture;

            u32 quadIndexCount = 0;
            QuadVertex* quadBuffer = NULL;
            QuadVertex* quadBufferRef = NULL;

            u32 circleIndexCount = 0;
            CircleVertex* circleBuffer = NULL;
            CircleVertex* circleBufferRef = NULL;

            Texture textureSlots[k_MaxTextures];
            u32 textureSlotIndex = 1;

            glm::vec4 quadVertexPositions[4];
            glm::vec4 circleVertexPositions[4];

            u32 drawCount = 0;
            u32 quadCount = 0;
            u32 circleCount = 0;
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
            void AddQuadToBatch(const glm::mat4& transform, Rectangle& source, float textureIndex, const glm::vec2& textureSize, const glm::vec3& color);
            void AddCircleToBatch(const glm::mat4& transform, const glm::vec3& color, float thickness, float fade);

            void Initialize()
            {
                if (isInitialized)
                {
                    WARN("Cannot initialize the renderer more than once");
                    return;
                }

                ASSERT(SDL_Init(SDL_INIT_VIDEO) != false, "Failed to initialize SDL3!");

                const ApplicationConfig config = Application::GetConfig();
                Window::Initialize(config.virtualWidth, config.virtualHeight, config.name.c_str());

                gladLoadGL();
                INFO("OpenGL functions have successfully loaded");
                INFO("GPU vendor: %s", glGetString(GL_VENDOR));
                INFO("GPU specs: %s", glGetString(GL_RENDERER));
                INFO("OpenGL version: %s", glGetString(GL_VERSION));

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                state.viewMatrix = glm::mat4(1.f);
                state.projectionMatrix = glm::mat4(1.f);

                state.defaultShader = Shaders::Load("assets/shaders/Default_vs.glsl", "assets/shaders/Default_fs.glsl");
                Shaders::CreateUniform(state.defaultShader, "viewMatrix");
                Shaders::CreateUniform(state.defaultShader, "projectionMatrix");
                Shaders::CreateUniform(state.defaultShader, "textures");

                state.circleShader = Shaders::Load("assets/shaders/BatchingCircles_vs.glsl", "assets/shaders/BatchingCircles_fs.glsl");
                Shaders::CreateUniform(state.circleShader, "viewMatrix");
                Shaders::CreateUniform(state.circleShader, "projectionMatrix");

                SetupBatchRendering();

                INFO("The renderer was successfully initialized");
                isInitialized = true;
            }

            void Shutdown()
            {
                INFO("The renderer is shutting down...");
                CleanUpBatchRendering();
                Shaders::Unload(state.defaultShader);
                Shaders::Unload(state.circleShader);
                Window::Shutdown();
                SDL_Quit();
            }

            void BeginScene2D(const Camera2D& camera)
            {
                const ApplicationConfig& config = Application::GetConfig();
                state.viewMatrix = Cameras::GetViewMatrix2D(camera);
                state.projectionMatrix = Cameras::GetProjectionMatrix2D();

                Shaders::Bind(state.defaultShader);
                Shaders::SetUniform(state.defaultShader, "viewMatrix", state.viewMatrix);
                Shaders::SetUniform(state.defaultShader, "projectionMatrix", state.projectionMatrix);

                Shaders::Bind(state.circleShader);
                Shaders::SetUniform(state.circleShader, "viewMatrix", state.viewMatrix);
                Shaders::SetUniform(state.circleShader, "projectionMatrix", state.projectionMatrix);

                batchData.quadCount = 0;
                batchData.circleCount = 0;
                batchData.drawCount = 0;

                BeginBatchQuad();
                BeginBatchCircle();
            }

            void EndScene2D()
            {
                EndBatchQuad();
                EndBatchCircle();

                Flush(BatchMode::Quads);
                Flush(BatchMode::Circles);
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

            void Flush(BatchMode mode)
            {
                if (batchData.quadIndexCount > 0 && mode == BatchMode::Quads)
                {
                    for (u32 i = 0; i < batchData.textureSlotIndex; i++)
                        Textures::Bind(batchData.textureSlots[i], i);

                    Shaders::Bind(state.defaultShader);
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
            }

            void DrawRectangle(const glm::vec2& position, const glm::vec2& size, const glm::vec3& color)
            {
                Rectangle rectangle;
                rectangle.x = position.x;
                rectangle.y = position.y;
                rectangle.width = size.x;
                rectangle.height = size.y;

                DrawRectanglePro(rectangle, glm::vec2(0.f), 0.f, color);
            }

            void DrawRectangleRec(const Rectangle& rectangle, const glm::vec3& color)
            {
                DrawRectanglePro(rectangle, glm::vec2(0.f), 0.f, color);
            }

            void DrawRectanglePro(const Rectangle& rectangle, const glm::vec2& origin, float rotation, const glm::vec3& color)
            {
                CheckForNewBatch(BatchMode::Quads);

                const glm::vec2 position = glm::vec2(rectangle.x, rectangle.y);
                const glm::vec2 size = glm::vec2(rectangle.width, rectangle.height);
                const glm::mat4 transform = Utils::GetTransfomMatrix2D(position, size, rotation, origin);
                const float textureIndex = 0.f;

                Rectangle source;
                source.width = 1.f;
                source.height = 1.f;

                AddQuadToBatch(transform, source, textureIndex, glm::vec2(1.f), color);
            }

            void DrawTexture(Texture& texture, const glm::vec2& position, const glm::vec3& tint)
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

            void DrawTextureEx(Texture& texture, const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec3& tint)
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
                    dest.width = texture.width;   // 64 Default White
                    dest.height = texture.height; // 64 Default White
                }

                glm::vec2 origin;
                origin.x = texture.width * scale.x / 2.f;
                origin.y = texture.height * scale.y / 2.f;

                DrawTexturePro(texture, source, dest, origin, rotation, tint);
            }

            void DrawTextureRec(Texture& texture, Rectangle& source, const glm::vec2& position, const glm::vec3& tint)
            {
                Rectangle dest;
                dest.x = position.x;
                dest.y = position.y;
                dest.width = texture.width;
                dest.height = texture.height;

                DrawTexturePro(texture, source, dest, glm::vec2(0.f), 0.f, tint);
            }

            void DrawTexturePro(Texture& texture, Rectangle& source, Rectangle& dest, const glm::vec2& origin, float rotation, const glm::vec3& tint)
            {
                CheckForNewBatch(BatchMode::Quads);

                const glm::vec2 position = glm::vec2(dest.x, dest.y);
                const glm::vec2 size = glm::vec2(dest.width, dest.height);
                const glm::vec2 textureSize = glm::vec2(texture.width, texture.height);
                const float textureIndex = CheckBatchForTextureIndex(texture);
                const glm::mat4 transform = Utils::GetTransfomMatrix2D(position, size, rotation, origin);

                AddQuadToBatch(transform, source, textureIndex, textureSize, tint);
            }

            void DrawCircle(const glm::vec2& center, float radius, const glm::vec3& color)
            {
                DrawCirclePro(center, radius, 1.f, 0.05f, color);
            }

            void DrawCirclePro(const glm::vec2& center, float radius, float thickness, float fade, const glm::vec3& color)
            {
                CheckForNewBatch(BatchMode::Circles);

                const glm::vec2 size = glm::vec2(64.f * radius);
                const glm::mat4 transform = Utils::GetTransfomMatrix2D(center, size, 0.f, glm::vec2(0.f));

                AddCircleToBatch(transform, color, thickness, fade);
            }

            glm::vec3& GetClearColor() { return state.clearColor; }
            u32 GetQuadCount() { return batchData.quadCount; }
            u32 GetCircleCount() { return batchData.circleCount; }
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
                glGenVertexArrays(1, &batchData.quadVertexArray);
                glGenBuffers(1, &batchData.quadVertexBuffer);
                glGenBuffers(1, &batchData.indexBuffer);

                glBindVertexArray(batchData.quadVertexArray);

                glBindBuffer(GL_ARRAY_BUFFER, batchData.quadVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, k_MaxVertexCount * sizeof(QuadVertex), NULL, GL_DYNAMIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(QuadVertex), (void*)offsetof(QuadVertex, position));

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(QuadVertex), (void*)offsetof(QuadVertex, color));

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(QuadVertex), (void*)offsetof(QuadVertex, texCoord));

                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(QuadVertex), (void*)offsetof(QuadVertex, texIndex));

                batchData.quadBuffer = new QuadVertex[k_MaxVertexCount];

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

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batchData.indexBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, k_MaxIndexCount * sizeof(u32), indices, GL_STATIC_DRAW);

                delete[] indices;

                // Circles
                glGenVertexArrays(1, &batchData.circleVertexArray);
                glGenBuffers(1, &batchData.circleVertexBuffer);

                glBindVertexArray(batchData.circleVertexArray);

                glBindBuffer(GL_ARRAY_BUFFER, batchData.circleVertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, k_MaxVertexCount * sizeof(CircleVertex), NULL, GL_DYNAMIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(CircleVertex), (void*)offsetof(CircleVertex, worldPosition));

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(CircleVertex), (void*)offsetof(CircleVertex, localPosition));

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 3, GL_FLOAT, false, sizeof(CircleVertex), (void*)offsetof(CircleVertex, color));

                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(CircleVertex), (void*)offsetof(CircleVertex, thickness));

                glEnableVertexAttribArray(4);
                glVertexAttribPointer(4, 1, GL_FLOAT, false, sizeof(CircleVertex), (void*)offsetof(CircleVertex, fade));

                batchData.circleBuffer = new CircleVertex[k_MaxVertexCount];

                // All
                s32 samplers[k_MaxTextures];
                for (u32 i = 0; i < k_MaxTextures; i++)
                    samplers[i] = i;

                Shaders::Bind(state.defaultShader);
                Shaders::SetUniform(state.defaultShader, "textures", samplers, k_MaxTextures);

                batchData.whiteTexture = Textures::LoadDefaultWhite();
                batchData.textureSlots[0] = batchData.whiteTexture;

                batchData.quadVertexPositions[0] = glm::vec4(0.f, 0.f, 0.f, 1.f);
                batchData.quadVertexPositions[1] = glm::vec4(1.f, 0.f, 0.f, 1.f);
                batchData.quadVertexPositions[2] = glm::vec4(1.f, 1.f, 0.f, 1.f);
                batchData.quadVertexPositions[3] = glm::vec4(0.f, 1.f, 0.f, 1.f);

                batchData.circleVertexPositions[0] = glm::vec4(-0.5f, -0.5f, 0.f, 1.f);
                batchData.circleVertexPositions[1] = glm::vec4(0.5f, -0.5f, 0.f, 1.f);
                batchData.circleVertexPositions[2] = glm::vec4(0.5f, 0.5f, 0.f, 1.f);
                batchData.circleVertexPositions[3] = glm::vec4(-0.5f, 0.5f, 0.f, 1.f);
            }

            void CleanUpBatchRendering()
            {
                delete[] batchData.quadBuffer;
                delete[] batchData.circleBuffer;

                glDeleteVertexArrays(1, &batchData.quadVertexArray);
                glDeleteBuffers(1, &batchData.quadVertexBuffer);

                glDeleteVertexArrays(1, &batchData.circleVertexArray);
                glDeleteBuffers(1, &batchData.circleVertexBuffer);

                glDeleteBuffers(1, &batchData.indexBuffer);

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

            void AddQuadToBatch(const glm::mat4& transform, Rectangle& source, float textureIndex, const glm::vec2& textureSize, const glm::vec3& color)
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
                    batchData.quadBufferRef->texIndex = textureIndex;
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
                    batchData.circleBufferRef++;
                }

                batchData.circleIndexCount += 6;
                batchData.circleCount++;
            }
        }
    }
}
