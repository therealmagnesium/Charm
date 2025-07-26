#include "SceneHeirarchyPanel.h"

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
            ASSERT(state.context != NULL, "The scene heirarchy must have a context to display!");

            ImGui::ShowDemoWindow();
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
            ImGui::PopID();
        }
    }
}
