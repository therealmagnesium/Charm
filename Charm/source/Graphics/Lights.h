#pragma once
#include <glm/vec3.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct Shader;

        struct DirectionalLight
        {
            glm::vec3 direction = glm::vec3(-0.2f, -0.8f, -0.6);
            glm::vec3 color = glm::vec3(1.f);
            float intensity = 1.f;
            Shader* shader = NULL;
        };

        namespace Lights
        {
            void UpdateUniforms(const DirectionalLight& sun);
        }
    }
}
