#include "Graphics/Renderer.h"
#include "Graphics/Camera.h"
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
        const static u32 k_MaxQuadCount = 1000;
        const static u32 k_MaxVertexCount = k_MaxQuadCount * 4;
        const static u32 k_MaxIndexCount = k_MaxQuadCount * 6;
        const static u32 k_MaxTextures = 32;

        struct BatchData
        {
            u32 vertexArray = 0;
            u32 vertexBuffer = 0;
            u32 indexBuffer = 0;

            u32 whiteTexture = 0;
            u32 whiteTextureSlot = 0;

            u32 indexCount = 0;

            Vertex* quadBuffer = NULL;
            Vertex* quadBufferRef = NULL;

            u32 textureSlots[k_MaxTextures];
            u32 textureSlotIndex = 1;
        };

        static RenderState state;
        static BatchData batchData;
        static bool isInitialized = false;

        namespace Renderer
        {
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
                INFO("The renderer was successfully initialized");
                INFO("GPU vendor: %s", glGetString(GL_VENDOR));
                INFO("GPU specs: %s", glGetString(GL_RENDERER));
                INFO("OpenGL version: %s", glGetString(GL_VERSION));

                state.viewMatrix = glm::mat4(1.f);
                state.projectionMatrix = glm::mat4(1.f);

                state.defaultShader.Load("assets/shaders/Default_vs.glsl", "assets/shaders/Default_fs.glsl");
                state.defaultShader.CreateUniform("viewMatrix");
                state.defaultShader.CreateUniform("projectionMatrix");

                batchData.quadBuffer = new Vertex[k_MaxVertexCount];

                isInitialized = true;
            }

            void Shutdown()
            {
                INFO("The renderer is shutting down...");
                delete[] batchData.quadBuffer;

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
            }

            void EndScene2D()
            {
            }

            glm::vec3& GetClearColor() { return state.clearColor; }

            void SetClearColor(float r, float g, float b)
            {
                state.clearColor.r = r;
                state.clearColor.g = g;
                state.clearColor.b = b;
            }
        }
    }
}
