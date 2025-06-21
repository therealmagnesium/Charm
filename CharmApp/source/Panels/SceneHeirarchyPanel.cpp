#include "SceneHeirarchyPanel.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Charm::ECS;

namespace Charm
{
    static SceneHeirarchyState state;

    namespace SceneHeirarchyPanel
    {
        void DrawEntityNode(Entity& entity);
        void DrawComponents(Entity& entity);
        void DrawVec3Control(const char* label, glm::vec3& v, float resetValue = 0.f, float columnWidth = 80.f);

        template <typename T, typename UIFunction>
        void DrawComponent(const char* name, Entity entity, UIFunction callback);

        void Display()
        {
            ASSERT(state.context != NULL, "The scene heirarchy must have a context to display!");

            ImGui::Begin("Scene Heirarchy");

            for (auto entityID : state.context->registry.view<entt::entity>())
            {
                Entity entity = Entities::Create(entityID, state.context);
                DrawEntityNode(entity);
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
                state.selectionContext = (Entity){};

            if (ImGui::BeginPopupContextWindow(NULL, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create a blank entity"))
                    Scenes::CreateEntity(*state.context);

                ImGui::EndPopup();
            }

            ImGui::End();

            ImGui::Begin("Inspector");

            if (state.selectionContext)
                DrawComponents(state.selectionContext);

            ImGui::End();
        }

        void SetContext(Scene* context) { state.context = context; }

        void DrawEntityNode(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            ImGuiTreeNodeFlags flags = ((state.selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            std::string entityLabel = internal.tag + "##" + std::to_string(internal.id);
            bool isOpen = ImGui::TreeNodeEx(entityLabel.c_str(), flags);

            if (ImGui::IsItemClicked())
                state.selectionContext = entity;

            bool shouldDeleteEntity = false;
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete"))
                    shouldDeleteEntity = true;

                ImGui::EndPopup();
            }

            if (isOpen)
                ImGui::TreePop();

            if (shouldDeleteEntity)
            {
                if (state.selectionContext == entity)
                    state.selectionContext = (Entity){};

                Scenes::DestroyEntity(*state.context, entity);
            }
        }

        void DrawComponents(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            char tagBuffer[256];
            memset(tagBuffer, 0, sizeof(tagBuffer));
            strncpy(tagBuffer, internal.tag.c_str(), internal.tag.size());

            if (ImGui::InputText("##Tag", tagBuffer, sizeof(tagBuffer)))
                internal.tag = std::string(tagBuffer);

            ImGui::SameLine();

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            if (ImGui::Button("Add component", ImVec2(-1.f, lineHeight)))
                ImGui::OpenPopup("Add Component");

            if (ImGui::BeginPopup("Add Component"))
            {
                if (ImGui::MenuItem("Transform"))
                {
                    state.selectionContext.AddComponent<TransformComponent>();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component) {
                /*
                        ImGui::DragFloat3("Position", glm::value_ptr(component.position));
                        ImGui::DragFloat3("Rotation", glm::value_ptr(component.rotation));
                        ImGui::DragFloat3("Scale", glm::value_ptr(component.scale));*/

                DrawVec3Control("Position", component.position);
                DrawVec3Control("Rotation", component.rotation);
                DrawVec3Control("Scale", component.scale, 1.f);
            });
        }

        void DrawVec3Control(const char* label, glm::vec3& v, float resetValue, float columnWidth)
        {
            ImGui::PushID(label);
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            ImGui::PushItemWidth(100.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            ImVec2 buttonSize = ImVec2(lineHeight + 3.f, lineHeight);

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.f, 0.05f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.f, 0.f, 1.f));
            if (ImGui::Button("X", buttonSize))
                v.x = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##X", &v.x, 1.f, 0.f, 0.f, "%.2f");

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.1f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.6f, 0.f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.5f, 0.f, 1.f));
            ImGui::SameLine();
            if (ImGui::Button("Y", buttonSize))
                v.y = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Y", &v.y, 1.f, 0.f, 0.f, "%.2f");

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.15f, 0.8f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.05f, 0.7f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.6f, 1.f));
            ImGui::SameLine();
            if (ImGui::Button("Z", buttonSize))
                v.z = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Z", &v.z, 1.f, 0.f, 0.f, "%.2f");

            ImGui::PopItemWidth();

            ImGui::PopStyleVar();
            ImGui::Columns(1);
            ImGui::PopID();
        }

        template <typename T, typename UIFunction>
        void DrawComponent(const char* name, Entity entity, UIFunction callback)
        {
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap |
                                             ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

            if (entity.HasComponent<T>())
            {
                auto& component = entity.GetComponent<T>();
                ImVec2 availableRegion = ImGui::GetContentRegionAvail();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.f, 4.f));

                float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
                ImGui::Separator();

                bool isOpen = ImGui::TreeNodeEx(name, flags);
                ImGui::PopStyleVar();

                ImGui::SameLine(availableRegion.x - lineHeight * 0.5f);
                if (ImGui::Button("+", ImVec2(lineHeight, lineHeight)))
                    ImGui::OpenPopup("Component Settings");

                bool removeComponent = false;
                if (ImGui::BeginPopup("Component Settings"))
                {
                    if (ImGui::MenuItem("Remove component"))
                        removeComponent = true;

                    ImGui::EndPopup();
                }

                if (isOpen)
                {
                    callback(component);
                    ImGui::TreePop();
                }

                if (removeComponent)
                    entity.RemoveComponent<T>();
            }
        }
    }
}
