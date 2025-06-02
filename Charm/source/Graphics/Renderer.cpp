#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
#include "Graphics/Texture.h"
#include "Graphics/Window.h"

#include "Core/Application.h"
#include "Core/Log.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <SDL3/SDL.h>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        const static u32 k_MaxQuadCount = 10000;
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

                state.defaultShader.Load("assets/shaders/Default_vs.glsl", "assets/shaders/Default_fs.glsl");
                state.defaultShader.CreateUniform("viewMatrix");
                state.defaultShader.CreateUniform("projectionMatrix");
                state.defaultShader.CreateUniform("textures");

                SetupBatchRendering();

                INFO("The renderer was successfully initialized");
                isInitialized = true;
            }

            void Shutdown()
            {
                INFO("The renderer is shutting down...");
                CleanUpBatchRendering();
                Window::Shutdown();
                SDL_Quit();
            }

            void BeginScene2D(const Camera2D& camera)
            {
                const ApplicationConfig& config = Application::GetConfig();
                state.viewMatrix = Cameras::GetViewMatrix(camera);
                state.projectionMatrix = glm::ortho(0.f, (float)config.virtualWidth, (float)config.virtualHeight, 0.f, -1.f, 1.f);

                state.defaultShader.Bind();
                state.defaultShader.SetUniform("viewMatrix", state.viewMatrix);
                state.defaultShader.SetUniform("projectionMatrix", state.projectionMatrix);

                batchData.textureSlotIndex = 1;
                batchData.quadCount = 0;
                batchData.drawCount = 0;
                batchData.indexCount = 0;

                BeginBatch();
            }

            void EndScene2D()
            {
                EndBatch();
                Flush();
            }

            void BeginBatch()
            {
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

            void DrawRectangle(float x, float y, float width, float height, const glm::vec3& color)
            {
                CheckForNewBatch();

                const float textureIndex = 0.f;

                batchData.quadBufferRef->position = glm::vec3(x, y, 0.f);
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2(0.f, 0.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = glm::vec3(x + width, y, 0.f);
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2(1.f, 0.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = glm::vec3(x + width, y + height, 0.f);
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2(1.f, 1.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = glm::vec3(x, y + height, 0.f);
                batchData.quadBufferRef->color = color;
                batchData.quadBufferRef->texCoord = glm::vec2(0.f, 1.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.indexCount += 6;
                batchData.quadCount++;
            }

            void DrawTexture(Texture& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec3& tint)
            {
                CheckForNewBatch();

                float textureIndex = 0.f;

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

                batchData.quadBufferRef->position = glm::vec3(position.x, position.y, 0.f);
                batchData.quadBufferRef->color = tint;
                batchData.quadBufferRef->texCoord = glm::vec2(0.f, 0.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = glm::vec3(position.x + size.x, position.y, 0.f);
                batchData.quadBufferRef->color = tint;
                batchData.quadBufferRef->texCoord = glm::vec2(1.f, 0.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = glm::vec3(position.x + size.x, position.y + size.y, 0.f);
                batchData.quadBufferRef->color = tint;
                batchData.quadBufferRef->texCoord = glm::vec2(1.f, 1.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.quadBufferRef->position = glm::vec3(position.x, position.y + size.y, 0.f);
                batchData.quadBufferRef->color = tint;
                batchData.quadBufferRef->texCoord = glm::vec2(0.f, 1.f);
                batchData.quadBufferRef->texIndex = textureIndex;
                batchData.quadBufferRef++;

                batchData.indexCount += 6;
                batchData.quadCount++;
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

                state.defaultShader.Bind();
                state.defaultShader.SetUniform("textures", samplers, k_MaxTextures);

                batchData.whiteTexture = Textures::LoadDefaultWhite();
                batchData.textureSlots[0] = batchData.whiteTexture;
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
        }
    }
}
