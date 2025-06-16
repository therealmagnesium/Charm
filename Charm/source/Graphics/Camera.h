#pragma once
#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct Camera2D
        {
            glm::vec2 target;
            glm::vec2 offset;
            float rotation = 0.f;
            float zoom = 1.f;
        };

        namespace Cameras
        {
            glm::mat4 GetViewMatrix2D(const Camera2D& camera);
            glm::mat4 GetProjectionMatrix2D();
        }
    }
}
