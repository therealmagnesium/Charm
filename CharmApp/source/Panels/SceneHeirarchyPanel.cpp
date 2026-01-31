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
        void DrawSceneHeirarchyPanel();
        void DrawCreateEntityPopup();
        void DrawEntityNode(Entity& entity);

        void Display()
        {
            if (state.shouldDisplay)
                DrawSceneHeirarchyPanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        Scene* GetContext() { return state.context; }
        Entity& GetSelectedEntity() { return state.selectionContext; }
        bool ShouldDisplay() { return state.shouldDisplay; }

        void SetContext(Scene& context)
        {
            state.context = &context;
            state.selectionContext = Entity_Null;
        }

        void SetSelectedEntity(const Entity& entity) { state.selectionContext = entity; }

        void DrawSceneHeirarchyPanel()
        {
            ASSERT(state.context != NULL, "SceneHierarchyPanel::Display - The scene heirarchy must have a context to display!");

            ImGui::Begin("Scene Heirarchy", &state.shouldDisplay);

            for (auto entityID : state.context->registry.view<entt::entity>())
            {
                Entity entity = Entities::Create(entityID, state.context);
                auto& internal = entity.GetComponent<InternalComponent>();
                if (!internal.parent)
                    DrawEntityNode(entity);
            }

            const ImVec2 windowSize = ImGui::GetWindowSize();
            const ImVec2 currentCursor = ImGui::GetCursorPos();
            const ImVec2 availableSpace = ImVec2(windowSize.x, windowSize.y - currentCursor.y);

            if (availableSpace.y > 0.f)
            {
                ImGui::InvisibleButton("##empty_space", availableSpace);

                if (ImGui::IsItemClicked())
                    state.selectionContext = Entity_Null;

                if (ImGui::BeginPopupContextWindow("Create Entity Popup", ImGuiPopupFlags_NoOpenOverExistingPopup | ImGuiPopupFlags_MouseButtonRight))
                {
                    DrawCreateEntityPopup();
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

        void DrawCreateEntityPopup()
        {
            if (ImGui::BeginMenu("Create"))
            {
                if (ImGui::MenuItem("Blank Entity"))
                    state.selectionContext = Scenes::CreateEntity(*state.context);

                ImGui::SeparatorText("2D");
                const bool shouldCreateSprite = ImGui::MenuItem("Sprite");
                const bool shouldCreateAnimatedSprite = ImGui::MenuItem("Animated Sprite");
                const bool shouldCreateCamera = ImGui::MenuItem("Camera");

                ImGui::SeparatorText("3D");
                const bool shouldCreateMesh = ImGui::MenuItem("Mesh");

                if (shouldCreateSprite)
                {
                    Entity entity = Scenes::CreateEntity(*state.context, "Sprite");
                    entity.AddComponent<SpriteRendererComponent>();
                    state.selectionContext = entity;
                }

                if (shouldCreateAnimatedSprite)
                {
                    Entity entity = Scenes::CreateEntity(*state.context, "Animated Sprite");
                    entity.AddComponent<SpriteRendererComponent>();
                    entity.AddComponent<Animator2DComponent>();
                    state.selectionContext = entity;
                }

                if (shouldCreateCamera)
                {
                    Entity entity = Scenes::CreateEntity(*state.context, "Camera");
                    entity.AddComponent<Camera2DComponent>();
                    state.selectionContext = entity;
                }

                if (shouldCreateMesh)
                {
                    Entity entity = Scenes::CreateEntity(*state.context, "Mesh");
                    entity.AddComponent<MeshRendererComponent>();
                    state.selectionContext = entity;
                }

                ImGui::EndMenu();
            }
        }

        void DrawEntityNode(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            ImGui::PushID(internal.id);
            ImGuiTreeNodeFlags flags = ((state.selectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            bool isOpen = false;

            const u32 childCount = Entities::GetChildCount(entity);
            if (childCount > 0)
            {
                ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 7.f);
                isOpen = ImGui::TreeNodeEx(internal.tag.c_str(), flags);
                ImGui::PopStyleVar();
            }
            else
            {
                const float indentWidth = 25.f;
                ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 4.f);
                ImGui::Indent(indentWidth);
                ImGui::Selectable(internal.tag.c_str(), state.selectionContext == entity);
                ImGui::Unindent(indentWidth);
                ImGui::PopStyleVar();
            }

            if (ImGui::IsItemClicked())
                state.selectionContext = entity;

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

            bool shouldDeleteEntity = false;
            if (ImGui::BeginPopupContextItem("Delete Entity Popup"))
            {
                if (ImGui::MenuItem("Duplicate"))
                    Scenes::DuplicateEntity(*entity.context, entity);

                if (ImGui::MenuItem("Delete"))
                    shouldDeleteEntity = true;

                ImGui::EndPopup();
            }

            ImGui::PopID();

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
