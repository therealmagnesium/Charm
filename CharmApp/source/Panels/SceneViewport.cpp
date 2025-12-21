#include "SceneViewport.h"
#include "SceneHeirarchyPanel.h"
#include "ToolbarPanel.h"
#include "../CharmApp.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>

using namespace Charm;
using namespace Charm::Graphics;
using namespace Charm::ECS;

namespace CharmApp
{
    static SceneViewportState state;

    namespace SceneViewportPanel
    {
        ImVec2 GetLargestViewportSize();
        ImVec2 GetCenteredViewportPosition(ImVec2& aspectSize);

        void Display(Framebuffer& framebuffer)
        {
            ImGui::Begin("Scene Viewport", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            state.isHovered = ImGui::IsWindowHovered();
            state.isFocused = ImGui::IsWindowFocused();

            Texture& displayTexture = framebuffer.colorAttachments[0];
            ImVec2 aspectSize = GetLargestViewportSize();
            ImVec2 windowPosition = GetCenteredViewportPosition(aspectSize);
            ImTextureID textureID = displayTexture.id;

            state.position = glm::vec2(ImGui::GetWindowPos().x + windowPosition.x, ImGui::GetWindowPos().y + windowPosition.y);
            state.size = *(glm::vec2*)&aspectSize;

            ImGui::SetCursorPos(windowPosition);
            ImGui::Image(textureID, aspectSize, ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));

            const SceneState activeSceneState = CharmApp::GetActiveSceneState();

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser Item");
                if (payload != NULL && activeSceneState == SceneState::Editor)
                {
                    std::filesystem::path path = (const char*)payload->Data;
                    path = ProjectManager::GetAssetFileSystemPath(path, CharmApp::GetProject());

                    std::string extension = path.extension().string();
                    if (extension == ".charm")
                        CharmApp::OpenScene(path);
                    else
                        ERROR("SceneViewportPanel::Display - Cannot load scene to viewport because it is not a \".charm\" file");
                }
                ImGui::EndDragDropTarget();
            }

            Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();
            if (selectedEntity.IsHandleValid() && activeSceneState == SceneState::Editor)
            {
                ImGuizmo::SetOrthographic(true);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(state.position.x, state.position.y, state.size.x, state.size.y);

                const glm::mat4 viewMatrix = Renderer::GetViewMatrix();
                const glm::mat4 projectionMatrix = Renderer::GetProjectionMatrix();

                auto& entityTransformComponent = selectedEntity.GetComponent<TransformComponent>();
                glm::mat4 entityTransform = Utils::GetTransfomMatrix2D(entityTransformComponent.position, entityTransformComponent.scale,
                                                                       entityTransformComponent.rotation.z, glm::vec2(0.f));

                const u32 manipulationType = ToolbarPanel::GetManipulationType();
                ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), (ImGuizmo::OPERATION)manipulationType, ImGuizmo::LOCAL, glm::value_ptr(entityTransform));

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 position, rotation, scale;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(entityTransform), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));
                    glm::vec3 deltaRotation = rotation - entityTransformComponent.rotation;

                    entityTransformComponent.position = position;
                    entityTransformComponent.rotation += deltaRotation;
                    entityTransformComponent.scale = scale;
                }
            }

            if (activeSceneState == SceneState::Editor)
            {
                Input::Capture(state.isHovered);

                if (Input::IsMouseClicked(MOUSE_BUTTON_LEFT) && !Input::IsKeyDown(KEY_LEFT_ALT) && state.isHovered && !ImGuizmo::IsUsing())
                {
                    const glm::vec2 glViewportMouse = Utils::ScreenToViewportGL(Input::GetMousePosition(),
                                                                                state.position,
                                                                                state.size);

                    Framebuffers::Bind(framebuffer);
                    const s32 pixelData = Framebuffers::ReadPixel(framebuffer, 1, (u32)glViewportMouse.x, (u32)glViewportMouse.y);
                    CharmApp::SetPixelData(pixelData);
                    Framebuffers::Unbind();

                    Scene* activeScene = CharmApp::GetActiveScene();
                    if (pixelData != -1)
                    {
                        Entity entity = Entities::Create((entt::entity)pixelData, activeScene);
                        SceneHeirarchyPanel::SetSelectedEntity(entity);
                    }
                    else if (pixelData == -1)
                        SceneHeirarchyPanel::SetSelectedEntity(Entity_Null);
                }
            }

            ImGui::End();
        }

        ImVec2 GetLargestViewportSize()
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();
            ImVec2 aspectSize = ImVec2(windowSize.x, windowSize.x / (16.f / 9.f));

            if (aspectSize.y > windowSize.y)
            {
                aspectSize.y = windowSize.y;
                aspectSize.x = windowSize.y * (16.f / 9.f);
            }

            return aspectSize;
        }

        ImVec2 GetCenteredViewportPosition(ImVec2& aspectSize)
        {
            ImVec2 windowSize = ImGui::GetContentRegionAvail();

            ImVec2 viewport;
            viewport.x = (windowSize.x / 2.f) - (aspectSize.x / 2.f) + ImGui::GetCursorPosX();
            viewport.y = (windowSize.y / 2.f) - (aspectSize.y / 2.f) + ImGui::GetCursorPosY();

            return viewport;
        }

        bool IsHovered() { return state.isHovered; }
        bool IsFocused() { return state.isFocused; }
        const glm::vec2& GetPosition() { return state.position; }
        const glm::vec2& GetSize() { return state.size; }
    }
}
