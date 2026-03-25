#include "SceneViewport.h"
#include "SceneHeirarchyPanel.h"
#include "ToolbarPanel.h"
#include "../CharmApp.h"

#include <imgui.h>
#include <imgui_internal.h>
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
        void Callback_PostProcessing(const ImDrawList*, const ImDrawCmd* command);

        void Display(Texture& renderTarget)
        {
            ImGui::Begin("Scene Viewport", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::SetKeyOwner(ImGuiMod_Alt, ImGui::GetID("Scene Viewport"));

            state.isHovered = ImGui::IsWindowHovered();
            state.isFocused = ImGui::IsWindowFocused();

            ImVec2 aspectSize = GetLargestViewportSize();
            ImVec2 windowPosition = GetCenteredViewportPosition(aspectSize);
            ImTextureID textureID = renderTarget.id;

            state.position = glm::vec2(ImGui::GetWindowPos().x + windowPosition.x, ImGui::GetWindowPos().y + windowPosition.y);
            state.size = *(glm::vec2*)&aspectSize;

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddCallback(Callback_PostProcessing, NULL);
            ImGui::SetCursorPos(windowPosition);
            ImGui::Image(textureID, aspectSize, ImVec2(0.f, 1.f), ImVec2(1.f, 0.f));
            drawList->AddCallback(ImDrawCallback_ResetRenderState, NULL);

            const SceneState activeSceneState = CharmApp::GetActiveSceneState();

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser Item");
                if (payload != NULL && activeSceneState == SceneState::Editor)
                {
                    std::filesystem::path path = (const char*)payload->Data;
                    path = ProjectManager::GetAssetFileSystemPath(path, ProjectManager::GetActive());

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
                const Project& project = ProjectManager::GetActive();
                const bool isOrthographic = project.type == ProjectType::TwoDimensional;
                ImGuizmo::SetOrthographic(isOrthographic);
                ImGuizmo::SetDrawlist();
                ImGuizmo::SetRect(state.position.x, state.position.y, state.size.x, state.size.y);

                const glm::mat4 viewMatrix = Renderer::GetViewMatrix();
                const glm::mat4 projectionMatrix = Renderer::GetProjectionMatrix();

                auto& entityTransformComponent = selectedEntity.GetComponent<TransformComponent>();

                glm::mat4 entityTransform = glm::mat4(1.f);

                if (isOrthographic)
                    entityTransform = Utils::GetTransformMatrix2D(entityTransformComponent.position, entityTransformComponent.scale,
                                                                  entityTransformComponent.rotation.z, glm::vec2(0.f));
                else
                    entityTransform = entityTransformComponent.GetMatrix3D();

                const u32 manipulationType = ToolbarPanel::GetManipulationType();
                ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix), (ImGuizmo::OPERATION)manipulationType, ImGuizmo::LOCAL, glm::value_ptr(entityTransform));

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 position, rotation, scale;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(entityTransform), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));

                    entityTransformComponent.position = position;
                    entityTransformComponent.rotation = rotation;
                    entityTransformComponent.scale = scale;
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

        void Callback_PostProcessing(const ImDrawList*, const ImDrawCmd* command)
        {
            const ImDrawData* drawData = ImGui::GetDrawData();
            float L = drawData->DisplayPos.x;
            float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
            float T = drawData->DisplayPos.y;
            float B = drawData->DisplayPos.y + drawData->DisplaySize.y;

            const float ortho[4][4] = {
                {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
                {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
                {0.0f, 0.0f, -1.0f, 0.0f},
                {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
            };

            const glm::mat4 projection = glm::make_mat4(&ortho[0][0]);
            const bool horizontalPass = CharmApp::IsBloomPassHorizontal();

            Shader& shader = Renderer::GetShaderPostProcessing();
            Shaders::Bind(shader);
            Textures::Bind(CharmApp::GetFramebufferHDR().colorAttachments[0], 0);
            Textures::Bind(CharmApp::GetFramebufferBloom(horizontalPass).colorAttachments[0], 1);

            Shaders::SetUniform(shader, "u_textureScreen", 0);
            Shaders::SetUniform(shader, "u_textureBloom", 1);
            Shaders::SetUniform(shader, "u_shouldBlur", false);
            Shaders::SetUniform(shader, "u_matrixProjection", projection);
            Shaders::SetUniform(shader, "u_exposure", Renderer::GetExposure());
        }

        bool IsHovered() { return state.isHovered; }
        bool IsFocused() { return state.isFocused; }
        const glm::vec2& GetPosition() { return state.position; }
        const glm::vec2& GetSize() { return state.size; }
    }
}
