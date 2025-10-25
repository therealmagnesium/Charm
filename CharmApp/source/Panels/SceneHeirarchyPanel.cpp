#include "SceneHeirarchyPanel.h"
#include "../CharmApp.h"

#include <Core/AssetManager.h>
#include <ECS/Components.h>
#include <Graphics/Texture.h>

#include <imgui.h>

using namespace Charm;
using namespace Charm::ECS;

namespace CharmApp
{
    static SceneHeirarchyState state;

    namespace SceneHeirarchyPanel
    {
        void DrawEntityNode(Entity& entity);

        void Display()
        {
            ASSERT(state.context != NULL, "SceneHierarchyPanel::Display - The scene heirarchy must have a context to display!");

            ImGui::ShowDemoWindow();
            ImGui::Begin("Scene Heirarchy");

            for (auto entityID : state.context->registry.view<entt::entity>())
            {
                Entity entity = Entities::Create(entityID, state.context);
                auto& internal = entity.GetComponent<InternalComponent>();
                if (!internal.parent)
                    DrawEntityNode(entity);
            }

            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 currentCursor = ImGui::GetCursorPos();
            ImVec2 availableSpace = ImVec2(windowSize.x, windowSize.y - currentCursor.y);

            if (availableSpace.y > 0)
            {
                ImGui::InvisibleButton("##empty_space", availableSpace);

                if (ImGui::IsItemClicked())
                    state.selectionContext = Entity_Null;

                if (ImGui::BeginPopupContextItem("Create Entity Popup", ImGuiPopupFlags_MouseButtonRight))
                {
                    if (ImGui::MenuItem("Create a blank entity"))
                        Scenes::CreateEntity(*state.context);

                    ImGui::EndPopup();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Scene Heirarchy Entity");
                    if (payload != NULL && CharmApp::GetActiveSceneState() == SceneState::Editor)
                    {
                        UUID childUUID = *(UUID*)payload->Data;
                        Entity child = Entities::FindWithUUID(childUUID, state.context);

                        if (child.IsHandleValid())
                        {
                            auto& childInternal = child.GetComponent<InternalComponent>();
                            childInternal.parent = Entity_Null; // Remove parent
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
            else
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
                    state.selectionContext = Entity_Null;
            }

            ImGui::End();
        }

        Scene* GetContext() { return state.context; }
        Entity& GetSelectedEntity() { return state.selectionContext; }

        void SetContext(Scene& context)
        {
            state.context = &context;
            state.selectionContext = (Entity){};
        }

        void SetSelectedEntity(const Entity& entity) { state.selectionContext = entity; }

        void DrawEntityNode(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            ImGui::PushID(internal.id);
            ImGuiTreeNodeFlags flags = ((state.selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            bool isOpen = ImGui::TreeNodeEx(internal.tag.c_str(), flags);

            bool shouldDeleteEntity = false;
            if (ImGui::BeginPopupContextItem("Delete Entity Popup"))
            {
                if (ImGui::MenuItem("Duplicate"))
                    Scenes::DuplicateEntity(*entity.context, entity);

                if (ImGui::MenuItem("Delete"))
                    shouldDeleteEntity = true;

                ImGui::EndPopup();
            }

            if (ImGui::IsItemClicked())
                state.selectionContext = entity;

            ImGui::PopID();

            if (ImGui::BeginDragDropSource())
            {
                if (state.selectionContext == entity)
                    ImGui::SetDragDropPayload("Scene Heirarchy Entity", &internal.id, sizeof(UUID));

                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Scene Heirarchy Entity");
                if (payload != NULL && CharmApp::GetActiveSceneState() == SceneState::Editor)
                {
                    UUID childUUID = *(UUID*)payload->Data;
                    UUID parentUUID = 0;

                    if (internal.parent)
                    {
                        auto& parentInternal = internal.parent.GetComponent<InternalComponent>();
                        parentUUID = parentInternal.id;
                    }

                    if (childUUID != internal.id && childUUID != parentUUID)
                    {
                        Entity child = Entities::FindWithUUID(childUUID, state.context);
                        auto& childInternal = child.GetComponent<InternalComponent>();
                        childInternal.parent = entity;
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (isOpen)
            {
                if (!shouldDeleteEntity)
                {
                    auto children = Entities::GetChildEntities(entity);
                    for (Entity& child : children)
                        DrawEntityNode(child);
                }

                ImGui::TreePop();
            }

            if (shouldDeleteEntity)
            {
                if (state.selectionContext == entity)
                    state.selectionContext = Entity_Null;

                auto children = Entities::GetChildEntities(entity);
                for (auto& child : children)
                    Scenes::DestroyEntity(*state.context, child);

                Scenes::DestroyEntity(*state.context, entity);
            }
        }
    }
}
