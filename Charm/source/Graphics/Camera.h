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

        struct Camera3D
        {
            glm::vec3 target;
            float fov = 45.f;
            float nearClip = 0.1f;
            float farClip = 500.f;
            float distance = 10.f;
            float yaw = 0.f;
            float pitch = 0.f;
        };

        using Camera = Camera3D;

        namespace Cameras
        {
            void UpdateEditor(Camera3D& camera);
            void UpdateRuntime(Camera3D& camera);

            glm::mat4 GetViewMatrix2D(const Camera2D& camera);
            glm::mat4 GetProjectionMatrix2D();

            glm::mat4 GetViewMatrix3D(const Camera3D& camera);
            glm::mat4 GetProjectionMatrix3D(const Camera3D& camera);

            glm::vec3 GetRightVector(const Camera3D& camera);
            glm::vec3 GetUpVector(const Camera3D& camera);
            glm::vec3 GetForwardVector(const Camera3D& camera);
            glm::vec3 CalculatePosition(const Camera3D& camera);
            glm::quat GetOrientation(const Camera3D& camera);
        }
    }
}
