#pragma once
#include <glm/glm.hpp>

namespace Charm
{
    namespace Graphics
    {
        struct Camera2D
        {
            glm::vec2 target = glm::vec2(0.f);
            glm::vec2 offset = glm::vec2(0.f);
            float rotation = 0.f;
            float zoom = 1.f;
        };

        struct EditorCamera3D
        {
            glm::vec3 target = glm::vec3(0.f);
            float fov = 45.f;
            float nearClip = 0.1f;
            float farClip = 500.f;
            float distance = 10.f;
            float yaw = 0.f;
            float pitch = 0.f;
        };

        struct SceneCamera3D
        {
            glm::vec3 position = glm::vec3(0.f);
            glm::vec3 rotation = glm::vec3(0.f);
            float fov = 45.f;
            float nearClip = 0.1f;
            float farClip = 500.f;
        };

        using EditorCamera = EditorCamera3D;
        using SceneCamera = SceneCamera3D;

        namespace Cameras
        {
            void UpdateEditor(Camera2D& camera);
            void UpdateEditor(EditorCamera3D& camera);

            glm::mat4 GetViewMatrix2D(const Camera2D& camera);
            glm::mat4 GetProjectionMatrix2D(const Camera2D& camera);

            glm::mat4 GetViewMatrix3D(const EditorCamera3D& camera);
            glm::mat4 GetProjectionMatrix3D(const EditorCamera3D& camera);

            glm::mat4 GetViewMatrix3D(const SceneCamera& camera);
            glm::mat4 GetProjectionMatrix3D(const SceneCamera& camera);

            glm::vec3 GetRightVector(const EditorCamera3D& camera);
            glm::vec3 GetUpVector(const EditorCamera3D& camera);
            glm::vec3 GetForwardVector(const EditorCamera3D& camera);
            glm::vec3 CalculatePosition(const EditorCamera3D& camera);
            glm::quat GetOrientation(const EditorCamera3D& camera);
        }

        inline const Camera2D Camera2D_Null;
        inline const EditorCamera3D EditorCamera3D_Null;
        inline const SceneCamera3D SceneCamera3D_Null;
    }
}
