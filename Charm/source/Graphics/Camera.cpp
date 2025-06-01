#include "Graphics/Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Charm
{
    namespace Graphics
    {
        namespace Cameras
        {
            glm::mat4 GetViewMatrix(const Camera2D& camera)
            {
                glm::mat4 transform = glm::mat4(1.f);
                transform = glm::translate(transform, glm::vec3(camera.target - camera.offset, 0.f));
                transform = glm::rotate(transform, glm::radians(camera.rotation), glm::vec3(0.f, 0.f, 1.f));

                glm::mat4 viewMatrix = glm::inverse(transform);
                return viewMatrix;
            }
        }
    }
}
