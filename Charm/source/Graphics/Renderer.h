#pragma once
#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct RenderState
        {
            glm::vec3 clearColor = glm::vec3(1.f);
        };

        namespace Renderer
        {
            void Initialize();
            void Shutdown();

            glm::vec3& GetClearColor();
            void SetClearColor(float r, float g, float b);
        }
    }
}
