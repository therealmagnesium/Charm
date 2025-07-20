#include "SceneHeirarchyPanel.h"
#include "../CharmApp.h"

#include <Core/AssetManager.h>
#include <Core/Log.h>
#include <Core/Utils.h>

#include <ECS/Components.h>

#include <Graphics/Texture.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

using namespace Charm;
using namespace Charm::ECS;

namespace CharmApp
{
    static SceneHeirarchyState state;

    namespace SceneHeirarchyPanel
    {
        void DrawEntityNode(Entity& entity);
        void DrawComponents(Entity& entity);
        void DrawFloatControl(const char* label, float* v, float min, float max, float columnWidth = 80.f);
        void DrawVec2Control(const char* label, glm::vec2& v, float speed, float resetValue = 0.f, float columnWidth = 70.f);
        void DrawVec3Control(const char* label, glm::vec3& v, float speed, float resetValue = 0.f, float columnWidth = 70.f);
        void DrawColorControl(const char* label, glm::vec3& v, float columnWidth = 50.f);

        template <typename T, typename UIFunction>
        void DrawComponent(const char* name, Entity entity, UIFunction callback);

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

            ImGui::Begin("Inspector");

            if (state.selectionContext)
                DrawComponents(state.selectionContext);

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

        void DrawComponents(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            char tagBuffer[256];
            memset(tagBuffer, 0, sizeof(tagBuffer));
            strncpy(tagBuffer, internal.tag.c_str(), internal.tag.size());

            ImGui::PushID("Is Active?");
            ImGui::Checkbox("##IsActive?", &internal.isActive);
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8f);
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

                if (ImGui::MenuItem("Circle Renderer"))
                {
                    state.selectionContext.AddComponent<CircleRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Sprite Renderer"))
                {
                    state.selectionContext.AddComponent<SpriteRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Camera 2D"))
                {
                    state.selectionContext.AddComponent<Camera2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Rigidbody 2D"))
                {
                    state.selectionContext.AddComponent<Rigidbody2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Box Collider 2D"))
                {
                    state.selectionContext.AddComponent<BoxCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Native Script"))
                {
                    state.selectionContext.AddComponent<NativeScriptComponent>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component) {
                const float columnWidth = 80.f;
                DrawVec3Control("Position", component.position, 0.1f, 0.f, columnWidth);
                DrawVec3Control("Rotation", component.rotation, 0.1f, 0.f, columnWidth);
                DrawVec3Control("Scale", component.scale, 0.1f, 1.f, columnWidth);
            });

            DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component) {
                const float columnWidth = 90.f;
                DrawFloatControl("Radius", &component.radius, 0.f, 100.f, columnWidth);
                DrawFloatControl("Thickness", &component.thickness, 0.f, 1.f, columnWidth);
                DrawFloatControl("Fade", &component.fade, 0.f, 1.f, columnWidth);
                DrawColorControl("Color", component.color, columnWidth);
            });

            DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component) {
                const float columnWidth = 115.f;
                ImGui::PushID("Texture");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Texture");
                ImGui::NextColumn();
                const AssetRegistry& registry = AssetManager::GetRegistry();
                std::string placeholder = (AssetManager::GetAsset<Texture>(component.sprite) != NULL) ? registry.at(component.sprite).path.c_str() : "Select texture";
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##Texture", placeholder.c_str()))
                {
                    if (ImGui::Selectable("None", component.sprite == 0))
                        component.sprite = 0;

                    for (auto& [handle, metadata] : registry)
                    {
                        const bool isSelected = (component.sprite == handle);
                        if (ImGui::Selectable(metadata.path.c_str(), isSelected))
                            component.sprite = handle;
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser Item");
                    if (payload != NULL && CharmApp::GetActiveSceneState() == SceneState::Editor)
                    {
                        std::filesystem::path path = (const char*)payload->Data;
                        path = ProjectManager::GetAssetFileSystemPath(path, CharmApp::GetProject());

                        std::string extension = path.extension().string();
                        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
                        {
                            AssetHandle handle = AssetManager::FindAssetHandle(path.string());
                            if (handle != 0)
                            {
                                AssetManager::Import(path.c_str(), AssetType::Texture, handle);
                                component.sprite = handle;
                            }
                            else
                                component.sprite = AssetManager::Import(path.c_str(), AssetType::Texture);

                            Texture* texture = AssetManager::GetAsset<Texture>(component.sprite);
                            component.crop.width = texture->width;
                            component.crop.height = texture->height;
                        }
                        else
                            ERROR("SceneHeirarchyPanel::Display - Cannot load texture because \"%s\" is not an image file", path.c_str());
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::Columns(1);
                ImGui::PopID();

                ImGui::PushID("Origin Mode");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Origin Mode");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##Origin Mode", Utils::OriginModeToString(component.originMode).c_str()))
                {
                    const char* modes[] = {"Center", "Left", "Right", "Bottom Left", "Bottom Middle", "Bottom Right", "Top Left", "Top Middle", "Top Right"};
                    for (u8 i = 0; i < LEN(modes); i++)
                    {
                        const bool isSelected = (modes[i] == Utils::OriginModeToString(component.originMode));
                        if (ImGui::Selectable(modes[i], isSelected))
                            component.originMode = Utils::StringToOriginMode(modes[i]);
                    }
                    ImGui::EndCombo();
                }
                ImGui::Columns(1);
                ImGui::PopID();

                ImGui::PushID("Sorting Layer");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Sorting Layer");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::InputInt("##Sorting Layer", &component.sortingLayer))
                {
                    if (component.sortingLayer < 0)
                        component.sortingLayer = 0;

                    if (component.sortingLayer >= MAX_SORTING_LAYERS)
                        component.sortingLayer = MAX_SORTING_LAYERS - 1;
                }
                ImGui::Columns(1);
                ImGui::PopID();

                ImGui::PushID("Crop");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0.f, columnWidth);
                ImGui::Text("Crop");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::DragFloat4("##Crop", &component.crop.x);
                ImGui::Columns(1);
                ImGui::PopID();

                DrawColorControl("Tint", component.tint, columnWidth);
            });

            DrawComponent<Camera2DComponent>("Camera 2D", entity, [](Camera2DComponent& component) {
                const float columnWidth = 100.f;
                DrawVec2Control("Offset", component.camera.offset, 0.1f, 0.f, columnWidth);
                DrawFloatControl("Zoom", &component.camera.zoom, 0.1f, 100.f, columnWidth);

                ImGui::PushID("Is Primary?");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Is Primary?");
                ImGui::NextColumn();

                ImGui::Checkbox("##", &component.isPrimary);
                ImGui::Columns(1);
                ImGui::PopID();
            });

            DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](Rigidbody2DComponent& component) {
                const float columnWidth = 130.f;
                ImGui::PushID("Body Type");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Body Type");
                ImGui::NextColumn();

                const char* types[3] = {"Static", "Dynamic", "Kinematic"};
                std::string preview = Utils::BodyTypeToString(component.type);
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##Body Type", preview.c_str()))
                {
                    for (u8 i = 0; i < LEN(types); i++)
                    {
                        const bool isSelected = (types[i] == preview.c_str());
                        if (ImGui::Selectable(types[i], isSelected))
                        {
                            component.type = (BodyType)i;
                            break;
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::Columns(1);
                ImGui::PopID();

                ImGui::PushID("Has Fixed Rotation");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Fixed Rotation?");
                ImGui::NextColumn();
                ImGui::Checkbox("##Fixed Rotation?", &component.hasFixedRotation);
                ImGui::Columns(1);
                ImGui::PopID();
            });

            DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](BoxCollider2DComponent& component) {
                const float columnWidth = 100.f;
                DrawVec2Control("Offset", component.offset, 0.1f, 0.f, columnWidth);
                DrawVec2Control("Size", component.size, 0.1f, 0.f, columnWidth);
                DrawFloatControl("Density", &component.density, 0.f, 0.f, columnWidth);
                DrawFloatControl("Friction", &component.friction, 0.f, 0.f, columnWidth);
                DrawFloatControl("Restitution", &component.restitution, 0.f, 0.f, columnWidth);
            });

            DrawComponent<NativeScriptComponent>("Native Script", entity, [](NativeScriptComponent& component) {
                ImGui::PushID("Script Name");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 110.f);
                ImGui::Text("Script Name");
                ImGui::NextColumn();

                std::string placeholder = (component.scriptName.empty()) ? "None" : component.scriptName;
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##Script Name", placeholder.c_str()))
                {
                    if (ImGui::Selectable("None", component.scriptName.empty()))
                        component.scriptName = "";

                    const ScriptBindingMap& bindings = ScriptManager::GetAllBindings();
                    for (auto& [name, binding] : bindings)
                    {
                        const bool isSelected = (component.scriptName == name);
                        if (ImGui::Selectable(name.c_str(), isSelected))
                            component.scriptName = name;
                    }

                    ImGui::EndCombo();
                }

                ImGui::Columns(1);
                ImGui::PopID();
            });
        }

        void DrawFloatControl(const char* label, float* v, float min, float max, float columnWidth)
        {
            ImGui::PushID(label);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::DragFloat("##", v, 0.01f, min, max);

            ImGui::Columns(1);
            ImGui::PopID();
        }

        void DrawVec2Control(const char* label, glm::vec2& v, float speed, float resetValue, float columnWidth)
        {
            ImGui::PushID(label);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            ImVec2 buttonSize = ImVec2(lineHeight + 3.f, lineHeight);

            ImGui::PushMultiItemsWidths(2, ImGui::GetContentRegionAvail().x - buttonSize.x * 2.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, ImGui::GetFontSize() / 4.f));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.f, 0.05f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.f, 0.f, 1.f));
            if (ImGui::Button("X", buttonSize))
                v.x = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##X", &v.x, speed, 0.f, 0.f);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.1f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.6f, 0.f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.5f, 0.f, 1.f));
            if (ImGui::Button("Y", buttonSize))
                v.y = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Y", &v.y, speed, 0.f, 0.f);
            ImGui::PopItemWidth();

            ImGui::PopStyleVar();
            ImGui::Columns(1);

            ImGui::PopID();
        }

        void DrawVec3Control(const char* label, glm::vec3& v, float speed, float resetValue, float columnWidth)
        {
            ImGui::PushID(label);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            ImVec2 buttonSize = ImVec2(lineHeight + 3.f, lineHeight);

            ImGui::PushMultiItemsWidths(3, ImGui::GetContentRegionAvail().x - buttonSize.x * 3.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, ImGui::GetFontSize() / 4.f));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.f, 0.05f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.f, 0.f, 1.f));
            if (ImGui::Button("X", buttonSize))
                v.x = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##X", &v.x, speed, 0.f, 0.f);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.1f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.6f, 0.f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.5f, 0.f, 1.f));
            if (ImGui::Button("Y", buttonSize))
                v.y = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Y", &v.y, speed, 0.f, 0.f);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.15f, 0.8f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.05f, 0.7f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.6f, 1.f));
            if (ImGui::Button("Z", buttonSize))
                v.z = resetValue;
            ImGui::PopStyleColor(3);

            ImGui::SameLine();
            ImGui::DragFloat("##Z", &v.z, speed, 0.f, 0.f);
            ImGui::PopItemWidth();

            ImGui::PopStyleVar();
            ImGui::Columns(1);

            ImGui::PopID();
        }

        void DrawColorControl(const char* label, glm::vec3& v, float columnWidth)
        {
            ImGui::PushID(label);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            ImGui::ColorEdit3("##", glm::value_ptr(v));

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
