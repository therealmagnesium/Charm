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
            u32 vertexArray = 0;
            u32 vertexBuffer = 0;
            u32 indexBuffer = 0;

            Texture whiteTexture;

            Vertex* quadBuffer = NULL;
            Vertex* quadBufferRef = NULL;

            Texture textureSlots[k_MaxTextures];
            u32 textureSlotIndex = 1;

            glm::vec4 quadVertexPositions[4];

            u32 indexCount = 0;
            u32 drawCount = 0;
            u32 quadCount = 0;
        };

        static RenderState state;
        static BatchData batchData;
        static bool isInitialized = false;

        namespace Renderer
        {
            void SetupBatchRendering();
            void CleanUpBatchRendering();
            void CheckForNewBatch();
            float CheckBatchForTextureIndex(Texture& texture);
            void AddQuadToBatch(const glm::mat4& transform, Rectangle& source, float textureIndex, const glm::vec2& textureSize, const glm::vec3& color);

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

                state.viewMatrix = glm::mat4(1.f);
                state.projectionMatrix = glm::mat4(1.f);

                state.defaultShader = Shaders::Load("assets/shaders/Default_vs.glsl", "assets/shaders/Default_fs.glsl");
                Shaders::CreateUniform(state.defaultShader, "viewMatrix");
                Shaders::CreateUniform(state.defaultShader, "projectionMatrix");
                Shaders::CreateUniform(state.defaultShader, "textures");

                SetupBatchRendering();

                INFO("The renderer was successfully initialized");
                isInitialized = true;
            }

            void Shutdown()
            {
                INFO("The renderer is shutting down...");
                CleanUpBatchRendering();
                Shaders::Unload(state.defaultShader);
                Window::Shutdown();
                SDL_Quit();
            }

            void BeginScene2D(const Camera2D& camera)
            {
                const ApplicationConfig& config = Application::GetConfig();
                state.viewMatrix = Cameras::GetViewMatrix(camera);
                state.projectionMatrix = glm::ortho(0.f, (float)config.virtualWidth, (float)config.virtualHeight, 0.f, -1.f, 1.f);

                Shaders::Bind(state.defaultShader);
                Shaders::SetUniform(state.defaultShader, "viewMatrix", state.viewMatrix);
                Shaders::SetUniform(state.defaultShader, "projectionMatrix", state.projectionMatrix);

                batchData.quadCount = 0;
                batchData.drawCount = 0;

                BeginBatch();
            }

            void EndScene2D()
            {
                EndBatch();
                Flush();
            }

            void BeginBatch()
            {
                batchData.indexCount = 0;
                batchData.textureSlotIndex = 1;
                batchData.quadBufferRef = batchData.quadBuffer;
            }

            void EndBatch()
            {
                u64 size = (u8*)batchData.quadBufferRef - (u8*)batchData.quadBuffer;

                glBindBuffer(GL_ARRAY_BUFFER, batchData.vertexBuffer);
                glBufferSubData(GL_ARRAY_BUFFER, 0, size, batchData.quadBuffer);
            }

            void Flush()
            {
                for (u32 i = 0; i < batchData.textureSlotIndex; i++)
                    Textures::Bind(batchData.textureSlots[i], i);

                glBindVertexArray(batchData.vertexArray);
                glDrawElements(GL_TRIANGLES, batchData.indexCount, GL_UNSIGNED_INT, NULL);

                batchData.drawCount++;
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
                CheckForNewBatch();

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

            void DrawTextureEx(Texture& texture, const glm::vec2& position, float rotation, float scale, const glm::vec3& tint)
            {
                Rectangle source;
                source.width = texture.width;
                source.height = texture.height;

                Rectangle dest;
                dest.x = position.x;
                dest.y = position.y;

                if (texture.id != 0)
                {
                    dest.width = texture.width * scale;
                    dest.height = texture.height * scale;
                }
                else
                {
                    dest.width = texture.width;   // 64 Default White
                    dest.height = texture.height; // 64 Default White
                }

                DrawTexturePro(texture, source, dest, glm::vec2(dest.width / 2.f), rotation, tint);
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
                CheckForNewBatch();

                const glm::vec2 position = glm::vec2(dest.x, dest.y);
                const glm::vec2 size = glm::vec2(dest.width, dest.height);
                const glm::vec2 textureSize = glm::vec2(texture.width, texture.height);
                const float textureIndex = CheckBatchForTextureIndex(texture);
                const glm::mat4 transform = Utils::GetTransfomMatrix2D(position, size, rotation, origin);

                AddQuadToBatch(transform, source, textureIndex, textureSize, tint);
            }

            glm::vec3& GetClearColor() { return state.clearColor; }
            u32 GetQuadCount() { return batchData.quadCount; }
            u32 GetDrawCount() { return batchData.drawCount; }

            void SetClearColor(float r, float g, float b)
            {
                state.clearColor.r = r;
                state.clearColor.g = g;
                state.clearColor.b = b;
            }

            void SetupBatchRendering()
            {
                batchData.quadBuffer = new Vertex[k_MaxVertexCount];

                glGenVertexArrays(1, &batchData.vertexArray);
                glGenBuffers(1, &batchData.vertexBuffer);
                glGenBuffers(1, &batchData.indexBuffer);

                glBindVertexArray(batchData.vertexArray);

                glBindBuffer(GL_ARRAY_BUFFER, batchData.vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, k_MaxVertexCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW);

                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, position));

                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, color));

                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

                glEnableVertexAttribArray(3);
                glVertexAttribPointer(3, 1, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texIndex));

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
            }

            void CleanUpBatchRendering()
            {
                delete[] batchData.quadBuffer;
                glDeleteVertexArrays(1, &batchData.vertexArray);
                glDeleteBuffers(1, &batchData.vertexBuffer);
                glDeleteBuffers(1, &batchData.indexBuffer);
                Textures::Unload(batchData.whiteTexture);
            }

            void CheckForNewBatch()
            {
                if (batchData.indexCount >= k_MaxIndexCount)
                {
                    EndBatch();
                    Flush();
                    BeginBatch();
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
                batchData.quadBufferRef->position = transform * batchData.quadVertexPositions[0];
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2(source.x / textureSize.x, source.y / textureSize.y);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = transform * batchData.quadVertexPositions[1];
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2((source.x + source.width) / textureSize.x, source.y / textureSize.y);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = transform * batchData.quadVertexPositions[2];
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2((source.x + source.width) / textureSize.x, (source.y + source.height) / textureSize.y);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = transform * batchData.quadVertexPositions[3];
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2(source.x / textureSize.x, (source.y + source.height) / textureSize.y);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.indexCount += 6;
                batchData.quadCount++;
            }
        }
    }
}
