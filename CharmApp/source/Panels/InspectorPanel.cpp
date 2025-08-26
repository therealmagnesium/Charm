#include "InspectorPanel.h"
#include "SceneHeirarchyPanel.h"
#include "ContentBrowserPanel.h"
#include "../CharmApp.h"

#include <Core/Utils.h>
#include <ECS/Components.h>
#include <UI/UI.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Charm;
using namespace Charm::Core;
using namespace Charm::ECS;

namespace CharmApp
{
    static InspectorState state;

    namespace InspectorPanel
    {
        void DrawComponents(Entity& entity);

        template <typename T, typename UIFunction>
        void DrawComponent(const char* name, Entity entity, UIFunction callback);

        void Display()
        {
            const std::filesystem::path path = ContentBrowserPanel::GetSelectedFilePath();
            const std::string stringPath = path.string();
            const std::string fileName = path.filename().string();
            Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();

            ImGui::Begin("Inspector");
            bool isAssetValid = AssetManager::IsHandleValid(state.selectedAssetHandle);
            state.selectedAsset = AssetManager::GetAsset(state.selectedAssetHandle);

            if (selectedEntity && stringPath.empty())
                DrawComponents(selectedEntity);

            if (isAssetValid && !selectedEntity && !stringPath.empty())
            {
                switch (state.selectedAsset->GetType())
                {
                    case AssetType::Texture:
                    {
                        Texture* texture = (Texture*)state.selectedAsset;
                        UI::DrawAssetControls_Texture(texture);
                        break;
                    }

                    case AssetType::Animation:
                    {
                        Animation* animation = (Animation*)state.selectedAsset;
                        UI::DrawAssetControls_Animation(animation);
                        break;
                    }

                    case AssetType::AnimationController:
                    {
                        AnimationController* controller = (AnimationController*)state.selectedAsset;
                        UI::DrawAssetControls_AnimationController(controller);
                        break;
                    }

                    default:
                        break;
                }
            }

            if (!isAssetValid && !selectedEntity && !stringPath.empty())
            {
                if (ImGui::Button("Add To Asset Registry"))
                {
                    const std::string extension = path.extension().string();
                    state.selectedAssetHandle = AssetManager::Import(stringPath.c_str(), Utils::ExtensionToAssetType(extension));
                }
            }

            ImGui::End();
        }

        void SetSelectedAsset(AssetHandle handle) { state.selectedAssetHandle = handle; }

        void DrawComponents(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            char tagBuffer[256];
            strncpy(tagBuffer, internal.tag.c_str(), internal.tag.size());

            ImGui::PushID("Is Active?");
            ImGui::Checkbox("##IsActive?", &internal.isActive);
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::CalcItemWidth() * 0.8f);
            if (ImGui::InputText("##Tag", tagBuffer, sizeof(tagBuffer)))
                internal.tag = std::string(tagBuffer);

            ImGui::SameLine();

            const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            if (ImGui::Button("Add component", ImVec2(-1.f, lineHeight)))
                ImGui::OpenPopup("Add Component");

            if (ImGui::BeginPopup("Add Component"))
            {
                if (ImGui::MenuItem("Transform"))
                {
                    entity.AddComponent<TransformComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Circle Renderer"))
                {
                    entity.AddComponent<CircleRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Sprite Renderer"))
                {
                    entity.AddComponent<SpriteRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Animator 2D"))
                {
                    entity.AddComponent<Animator2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Camera 2D"))
                {
                    entity.AddComponent<Camera2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Rigidbody 2D"))
                {
                    entity.AddComponent<Rigidbody2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Box Collider 2D"))
                {
                    entity.AddComponent<BoxCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (ImGui::MenuItem("Native Script"))
                {
                    entity.AddComponent<NativeScriptComponent>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            DrawComponent<TransformComponent>("Transform", entity, [](TransformComponent& component) {
                const float columnWidth = 80.f;
                UI::DrawVec3Control("Position", component.position, 0.1f, 0.f, columnWidth);
                UI::DrawVec3Control("Rotation", component.rotation, 0.1f, 0.f, columnWidth);
                UI::DrawVec3Control("Scale", component.scale, 0.1f, 1.f, columnWidth);
            });

            DrawComponent<CircleRendererComponent>("Circle Renderer", entity, [](CircleRendererComponent& component) {
                const float columnWidth = 110.f;
                UI::DrawFloatControl("Radius", &component.radius, 0.f, 100.f, columnWidth);
                UI::DrawFloatControl("Thickness", &component.thickness, 0.f, 1.f, columnWidth);
                UI::DrawFloatControl("Fade", &component.fade, 0.f, 1.f, columnWidth);
                UI::DrawColorControl("Color", component.color, columnWidth);
                UI::DrawIntInputControl("Sorting Layer", &component.sortingLayer, 0, MAX_SORTING_LAYERS, columnWidth);
            });

            DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](SpriteRendererComponent& component) {
                const float columnWidth = 115.f;
                ImGui::PushID("Texture");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Texture");
                ImGui::NextColumn();
                std::string placeholder = (AssetManager::GetAsset<Texture>(component.sprite) != NULL) ? AssetManager::GetAssetPath(component.sprite).stem().string() : "Select texture";
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##Texture", placeholder.c_str()))
                {
                    if (ImGui::Selectable("None", component.sprite == 0))
                        component.sprite = 0;

                    const AssetRegistry& registry = AssetManager::GetRegistry();
                    for (auto& [handle, metadata] : registry)
                    {
                        Asset* asset = AssetManager::GetAsset(handle);
                        if (asset->GetType() == AssetType::Texture)
                        {
                            const bool isSelected = (component.sprite == handle);
                            const std::string stemName = metadata.path.stem().string();
                            if (ImGui::Selectable(stemName.c_str(), isSelected))
                                component.sprite = handle;
                        }
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

                UI::DrawIntInputControl("Sorting Layer", &component.sortingLayer, 0, MAX_SORTING_LAYERS, columnWidth);

                ImGui::PushID("Crop");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0.f, columnWidth);
                ImGui::Text("Crop");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::DragFloat4("##Crop", &component.crop.x);
                ImGui::Columns(1);
                ImGui::PopID();

                UI::DrawColorControl("Tint", component.tint, columnWidth);
            });

            DrawComponent<Animator2DComponent>("Animator 2D", entity, [](Animator2DComponent& component) {
                const float columnWidth = 120.f;
                AnimationController* controller = AssetManager::GetAsset<AnimationController>(component.controller);
                std::string placeholder = (controller != NULL) ? AssetManager::GetAssetPath(component.controller).stem().string() : "Select Animation Controller";

                ImGui::PushID("Animation");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Animation");
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##", placeholder.c_str()))
                {
                    if (ImGui::Selectable("None", component.controller == 0))
                        component.controller = 0;

                    const AssetRegistry& registry = AssetManager::GetRegistry();
                    for (auto& [handle, metadata] : registry)
                    {
                        Asset* asset = AssetManager::GetAsset(handle);
                        if (asset->GetType() == AssetType::AnimationController)
                        {
                            const bool isSelected = (component.controller == handle);
                            const std::string stemName = metadata.path.stem().string();
                            if (ImGui::Selectable(stemName.c_str(), isSelected))
                                component.controller = handle;
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::Columns(1);
                ImGui::PopID();

                if (controller != NULL)
                    UI::DrawIntInputControl("Active Slot", &component.activeSlot, -1, controller->animations.size() - 1, columnWidth);
            });

            DrawComponent<Camera2DComponent>("Camera 2D", entity, [](Camera2DComponent& component) {
                const float columnWidth = 100.f;
                UI::DrawVec2Control("Offset", component.camera.offset, 0.1f, 0.f, columnWidth);
                UI::DrawFloatControl("Zoom", &component.camera.zoom, 0.1f, 100.f, columnWidth);
                UI::DrawBoolControl("Is Primary?", &component.isPrimary, columnWidth);
            });

            DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](Rigidbody2DComponent& component) {
                const float columnWidth = 150.f;
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
                            component.type = (PhysicsBodyType)i;
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

                UI::DrawFloatControl("Gravity Scale", &component.gravityScale, 0.f, 0.f, columnWidth);
                UI::DrawFloatControl("Linear Damping", &component.linearDamping, 0.f, 0.f, columnWidth);
                UI::DrawFloatControl("Angular Damping", &component.angularDamping, 0.f, 0.f, columnWidth);
            });

            DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](BoxCollider2DComponent& component) {
                const float columnWidth = 100.f;
                UI::DrawVec2Control("Offset", component.offset, 0.1f, 0.f, columnWidth);
                UI::DrawVec2Control("Size", component.size, 0.1f, 0.f, columnWidth);
                UI::DrawBoolControl("Is Trigger?", &component.isTrigger, columnWidth);
                UI::DrawFloatControl("Density", &component.density, 0.f, 0.f, columnWidth);
                UI::DrawFloatControl("Friction", &component.friction, 0.f, 1.f, columnWidth);
                UI::DrawFloatControl("Restitution", &component.restitution, 0.f, 1.f, columnWidth);
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

                const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
                // ImGui::Separator();

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
