#include "Graphics/Camera.h"

#include "Core/Application.h"
#include "Core/Input.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        namespace Cameras
        {
            void UpdateEditor(Camera2D& camera)
            {
                const auto& MousePan = [&](const glm::vec2& delta) {
                    const float panSpeed = 30.f / camera.zoom;
                    camera.target.x -= delta.x * panSpeed;
                    camera.target.y += delta.y * panSpeed;
                };

                const auto& CalculateZoomSpeed = [&]() {
                    float distance = camera.zoom * 0.35f;
                    distance = std::max(distance, 0.0f);
                    float speed = distance * distance;
                    speed = std::min(speed, 5.f);

                    return speed;
                };

                const auto& MouseZoom = [&](float delta) {
                    camera.zoom += delta * CalculateZoomSpeed();
                    if (camera.zoom < 0.1f)
                        camera.zoom = 0.1f;
                };

                if (Input::IsKeyDown(KEY_LEFT_ALT) && !ImGuizmo::IsOver())
                {
                    const glm::vec2 mouseDelta = Input::GetMouseRelative() * 0.003f;

                    if (Input::IsMouseDown(MOUSE_BUTTON_LEFT))
                        MousePan(mouseDelta);
                }

                const float scrollSpeed = Input::GetMouseScroll().y;
                if (glm::abs(scrollSpeed) > 0.f)
                    MouseZoom(scrollSpeed);
            }

            void UpdateEditor(EditorCamera3D& camera)
            {
                const auto& MousePan = [&](const glm::vec2& delta) {
                    const float panSpeed = 2.f * camera.distance;
                    camera.target += -GetRightVector(camera) * delta.x * panSpeed;
                    camera.target += GetUpVector(camera) * delta.y * panSpeed;
                };

                const auto& MouseRotate = [&](const glm::vec2& delta) {
                    const float rotationSpeed = glm::degrees(10.f);
                    float yawSign = GetUpVector(camera).y < 0 ? -1.0f : 1.0f;
                    camera.yaw -= yawSign * delta.x * rotationSpeed;
                    camera.pitch -= delta.y * rotationSpeed;
                };

                const auto& CalculateZoomSpeed = [&]() {
                    float distance = camera.distance * 0.1f;
                    distance = std::max(distance, 0.0f);
                    float speed = distance * distance;
                    speed = std::min(speed, 10.0f);

                    return speed;
                };

                const auto& MouseZoom = [&](float delta) {
                    camera.distance -= delta * CalculateZoomSpeed();
                    if (camera.distance < 1.0f)
                    {
                        camera.target += GetForwardVector(camera);
                        camera.distance = 1.0f;
                    }
                };

                if (Input::IsKeyDown(KEY_LEFT_ALT))
                {
                    glm::vec2 mouseDelta = Input::GetMouseRelative() * 0.003f;

                    if (Input::IsMouseDown(MOUSE_BUTTON_LEFT))
                        MousePan(mouseDelta);

                    if (Input::IsMouseDown(MOUSE_BUTTON_RIGHT))
                        MouseRotate(mouseDelta);

                    if (Input::IsMouseDown(MOUSE_BUTTON_MIDDLE))
                        MouseZoom(mouseDelta.y);
                }

                const float scrollSpeed = Input::GetMouseScroll().y;
                if (glm::abs(scrollSpeed) > 0.f)
                    MouseZoom(scrollSpeed);
            }

            glm::mat4 GetViewMatrix2D(const Camera2D& camera)
            {
                glm::mat4 transform = glm::mat4(1.f);
                transform = glm::translate(transform, glm::vec3(camera.target, 0.f));
                transform = glm::rotate(transform, glm::radians(camera.rotation), glm::vec3(0.f, 0.f, 1.f));
                transform = glm::translate(transform, glm::vec3(-camera.offset, 0.f));

                glm::mat4 viewMatrix = glm::inverse(transform);
                return viewMatrix;
            }

            glm::mat4 GetProjectionMatrix2D(const Camera2D& camera)
            {
                const ApplicationConfig& config = Application::GetConfig();
                glm::vec2 virtualSize;
                virtualSize.x = (float)config.virtualWidth / (float)Application::GetPixelsPerUnit();
                virtualSize.y = (float)config.virtualHeight / (float)Application::GetPixelsPerUnit();
                const glm::vec2 halfVirtualSize = virtualSize / 2.f;

                glm::vec2 horizontalBounds;
                glm::vec2 verticalBounds;

                horizontalBounds.x = -halfVirtualSize.x / camera.zoom;
                horizontalBounds.y = halfVirtualSize.x / camera.zoom;
                verticalBounds.x = -halfVirtualSize.y / camera.zoom;
                verticalBounds.y = halfVirtualSize.y / camera.zoom;

                glm::mat4 projectionMatrix = glm::mat4(1.f);
                projectionMatrix = glm::ortho(horizontalBounds.x, horizontalBounds.y, verticalBounds.x, verticalBounds.y, -10.f, 10.f);

                return projectionMatrix;
            }

            glm::mat4 GetViewMatrix3D(const EditorCamera3D& camera)
            {
                // viewMatrix = glm::lookAt(camera.position, camera.target, camera.up);

                glm::mat4 viewMatrix = glm::mat4(1.f);
                glm::quat orientation = GetOrientation(camera);
                glm::vec3 position = CalculatePosition(camera);
                viewMatrix = glm::translate(viewMatrix, position) * glm::toMat4(orientation);
                viewMatrix = glm::inverse(viewMatrix);

                return viewMatrix;
            }

            glm::mat4 GetProjectionMatrix3D(const EditorCamera3D& camera)
            {
                const ApplicationConfig& config = Application::GetConfig();
                const float aspectRatio = (float)config.virtualWidth / (float)config.virtualHeight;

                glm::mat4 projectionMatrix = glm::mat4(1.f);
                projectionMatrix = glm::perspective(glm::radians(camera.fov), aspectRatio, camera.nearClip, camera.farClip);

                return projectionMatrix;
            }

            glm::mat4 GetViewMatrix3D(const SceneCamera& camera)
            {
                const glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
                glm::vec3 forward;
                forward.x = cos(glm::radians(camera.rotation.x)) * cos(glm::radians(camera.rotation.y));
                forward.y = sin(glm::radians(camera.rotation.y));
                forward.z = sin(glm::radians(camera.rotation.x)) * cos(glm::radians(camera.rotation.y));
                forward = glm::normalize(forward);
                return glm::lookAt(camera.position, camera.position + forward, up);
            }

            glm::mat4 GetProjectionMatrix3D(const SceneCamera& camera)
            {
                const ApplicationConfig& config = Application::GetConfig();
                const float aspectRatio = (float)config.virtualWidth / (float)config.virtualHeight;

                glm::mat4 projectionMatrix = glm::mat4(1.f);
                projectionMatrix = glm::perspective(glm::radians(camera.fov), aspectRatio, camera.nearClip, camera.farClip);

                return projectionMatrix;
            }

            glm::vec3 GetRightVector(const EditorCamera3D& camera) { return glm::rotate(GetOrientation(camera), glm::vec3(1.f, 0.f, 0.f)); }
            glm::vec3 GetUpVector(const EditorCamera3D& camera) { return glm::rotate(GetOrientation(camera), glm::vec3(0.f, 1.f, 0.f)); }
            glm::vec3 GetForwardVector(const EditorCamera3D& camera) { return glm::rotate(GetOrientation(camera), glm::vec3(0.f, 0.f, -1.f)); }
            glm::vec3 CalculatePosition(const EditorCamera3D& camera) { return camera.target - GetForwardVector(camera) * camera.distance; }
            glm::quat GetOrientation(const EditorCamera3D& camera) { return glm::quat(glm::vec3(glm::radians(-camera.pitch), glm::radians(-camera.yaw), 0.f)); }
        }
    }
}
