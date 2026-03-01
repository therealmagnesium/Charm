#include "InspectorPanel.h"
#include "ContentBrowserPanel.h"
#include "SceneHeirarchyPanel.h"
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
        void DrawInspectorPanel();
        void DrawComponents(Entity& entity);

        template <typename T, typename UIFunction>
        void DrawComponent(const char* name, Entity entity, UIFunction callback);

        void Display()
        {
            if (state.shouldDisplay)
                DrawInspectorPanel();
        }

        void Toggle() { state.shouldDisplay = !state.shouldDisplay; }
        bool ShouldDisplay() { return state.shouldDisplay; }

        void SetSelectedAsset(AssetHandle handle) { state.selectedAssetHandle = handle; }

        void DrawInspectorPanel()
        {
            const std::filesystem::path path = ContentBrowserPanel::GetSelectedFilePath();
            Entity& selectedEntity = SceneHeirarchyPanel::GetSelectedEntity();

            ImGui::Begin("Inspector", &state.shouldDisplay);
            bool isAssetValid = AssetManager::IsHandleValid(state.selectedAssetHandle);
            state.selectedAsset = isAssetValid ? AssetManager::GetAsset(state.selectedAssetHandle) : NULL;

            if (selectedEntity && path.empty())
                DrawComponents(selectedEntity);

            if (isAssetValid && !selectedEntity && !path.empty())
            {
                switch (state.selectedAsset->GetType())
                {
                    case AssetType::Texture:
                    {
                        Texture* texture = (Texture*)state.selectedAsset;
                        UI::DrawAssetControls_Texture(texture);

                        if (ImGui::Button("Apply"))
                        {
                            Textures::Invalidate(*texture);

                            Scene* context = SceneHeirarchyPanel::GetContext();
                            auto sprites = context->registry.view<SpriteRendererComponent>();
                            for (auto entityID : sprites)
                            {
                                Entity entity = Entities::Create(entityID, context);
                                auto& spriteRenderer = entity.GetComponent<SpriteRendererComponent>();

                                if (texture->handle != spriteRenderer.sprite)
                                    continue;
                            }
                        }
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

                    case AssetType::TilePalette:
                    {
                        TilePalette* tilePalette = (TilePalette*)state.selectedAsset;
                        UI::DrawAssetControls_TilePalette(tilePalette);
                        break;
                    }

                    default:
                        break;
                }
            }

            const bool isValidFileSelected = !isAssetValid && !selectedEntity && !path.empty() && !ContentBrowserPanel::IsRenameActive();
            if (isValidFileSelected)
            {
                const std::string extension = path.extension().string();
                const AssetType type = Utils::ExtensionToAssetType(extension);
                if (type != AssetType::Invalid)
                {
                    if (ImGui::Button("Add To Asset Registry"))
                        state.selectedAssetHandle = AssetManager::Import(path, type);
                }
                else
                    ImGui::TextUnformatted("[UNSPECIFIED ASSET TYPE]");
            }

            ImGui::End();
        }

        void DrawComponents(Entity& entity)
        {
            auto& internal = entity.GetComponent<InternalComponent>();

            char tagBuffer[256];
            strncpy(tagBuffer, internal.tag.c_str(), internal.tag.size() + 1);

            ImGui::PushID("Is Active?");
            ImGui::Checkbox("##IsActive?", &internal.isActive);
            ImGui::PopID();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::InputText("##Tag", tagBuffer, sizeof(tagBuffer)))
                internal.tag = std::string(tagBuffer);

            const auto DrawTransformComponent = [](TransformComponent& component)
            {
                const float columnWidth = 80.f;
                UI::DrawVec3Control("Position", component.position, 0.1f, 0.f, columnWidth);
                UI::DrawVec3Control("Rotation", component.rotation, 0.1f, 0.f, columnWidth);
                UI::DrawVec3Control("Scale", component.scale, 0.1f, 1.f, columnWidth);
            };

            const auto DrawDirectionalLightComponent = [](DirectionalLightComponent& component)
            {
                const float columnWidth = 100.f;
                UI::DrawVec3Control("Direction", component.sun.direction, 0.01f, 1.f, columnWidth);
                UI::DrawFloatControl("Intensity", &component.sun.intensity, 0.f, 100.f, 0.01f, columnWidth);
                UI::DrawColorControl("Color", component.sun.color, columnWidth);
            };

            const auto DrawCircleRendererComponent = [](CircleRendererComponent& component)
            {
                const float columnWidth = 110.f;
                UI::DrawFloatControl("Radius", &component.radius, 0.f, 100.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Thickness", &component.thickness, 0.f, 1.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Fade", &component.fade, 0.f, 1.f, 0.01f, columnWidth);
                UI::DrawColorControl("Color", component.color, columnWidth);
                UI::DrawIntInputControl("Sorting Layer", &component.sortingLayer, 0, MAX_SORTING_LAYERS, columnWidth);
            };

            const auto DrawSpriteRendererComponent = [](SpriteRendererComponent& component)
            {
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

                    const std::vector<AssetHandle> textures = AssetManager::GetAllHandlesOfType(AssetType::Texture);
                    for (AssetHandle handle : textures)
                    {
                        Texture* texture = AssetManager::GetAsset<Texture>(handle);
                        const bool isSelected = (component.sprite == handle);
                        const std::string stemName = AssetManager::GetAssetPath(handle).stem().string();
                        if (ImGui::Selectable(stemName.c_str(), isSelected))
                        {
                            component.sprite = handle;
                            component.crop.width = texture->width;
                            component.crop.height = texture->height;
                        }
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser Item");
                    if (payload != NULL && CharmApp::GetActiveSceneState() == SceneState::Editor)
                    {
                        const std::filesystem::path path = (const char*)payload->Data;
                        const std::string extension = path.extension().string();

                        if (extension == ".png" || extension == ".jpg" || extension == ".jpeg")
                        {
                            const AssetHandle handle = AssetManager::FindAssetHandle(path);
                            if (handle != 0)
                                component.sprite = handle;
                            else
                                component.sprite = AssetManager::Import(path, AssetType::Texture);

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
                UI::DrawVec2Control("Tiling Factor", component.tilingFactor, 0.2f, 1.f, columnWidth);
                UI::DrawColorControl("Tint", component.tint, columnWidth);
            };

            const auto DrawMeshRendererComponent = [](MeshRendererComponent& component)
            {
                const float columnWidth = 60.f;
                ImGui::PushID("Mesh");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Mesh");
                ImGui::NextColumn();
                std::string placeholder = (AssetManager::GetAsset<Model>(component.model) != NULL) ? AssetManager::GetAssetPath(component.model).stem().string() : "Select mesh";
                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##Mesh", placeholder.c_str()))
                {
                    if (ImGui::Selectable("None", component.model == AssetHandle_Invalid))
                        component.model = AssetHandle_Invalid;

                    const std::vector<AssetHandle> models = AssetManager::GetAllHandlesOfType(AssetType::Model);
                    for (AssetHandle handle : models)
                    {
                        Model* model = AssetManager::GetAsset<Model>(handle);
                        const bool isSelected = (component.model == handle);
                        const std::string stemName = AssetManager::GetAssetPath(handle).stem().string();
                        if (ImGui::Selectable(stemName.c_str(), isSelected))
                            component.model = handle;
                    }

                    ImGui::EndCombo();
                }
                ImGui::Columns(1);
                ImGui::PopID();
            };

            const auto DrawAnimator2DComponent = [](Animator2DComponent& component)
            {
                const float columnWidth = 120.f;
                AnimationController* controller = AssetManager::GetAsset<AnimationController>(component.controller);
                std::string placeholder = (controller != NULL) ? AssetManager::GetAssetPath(component.controller).stem().string() : "Select Animation Controller";

                ImGui::PushID("Animation");
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, columnWidth);
                ImGui::Text("Controller");
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1.f);
                if (ImGui::BeginCombo("##", placeholder.c_str()))
                {
                    if (ImGui::Selectable("None", component.controller == 0))
                        component.controller = 0;

                    std::vector<AssetHandle> controllers = AssetManager::GetAllHandlesOfType(AssetType::AnimationController);
                    for (AssetHandle handle : controllers)
                    {
                        const bool isSelected = (component.controller == handle);
                        const std::string stemName = AssetManager::GetAssetPath(handle).stem().string();
                        if (ImGui::Selectable(stemName.c_str(), isSelected))
                            component.controller = handle;
                    }

                    ImGui::EndCombo();
                }

                if (ImGui::BeginDragDropTarget())
                {
                    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Content Browser Item");
                    if (payload != NULL && CharmApp::GetActiveSceneState() == SceneState::Editor)
                    {
                        const std::filesystem::path path = (const char*)payload->Data;
                        const std::string extension = path.extension().string();

                        if (extension == ".ac")
                        {
                            const AssetHandle handle = AssetManager::FindAssetHandle(path);
                            if (handle != 0)
                                component.controller = handle;
                            else
                                component.controller = AssetManager::Import(path, AssetType::AnimationController);
                        }
                        else
                            ERROR("SceneHeirarchyPanel::Display - Cannot load animation controller because \"%s\" is not an animation controller file (.ac)!", path.c_str());
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::Columns(1);
                ImGui::PopID();

                if (controller != NULL)
                {
                    UI::DrawIntInputControl("Active Slot", &component.activeSlot, -1, controller->animations.size() - 1, columnWidth);
                    if (controller->animations.size() < 1)
                        component.activeSlot = -1;
                }
            };

            const auto DrawCamera2DComponent = [](Camera2DComponent& component)
            {
                const float columnWidth = 105.f;
                Entity prevActiveCameraEntity = Scenes::GetActiveCameraEntity2D();

                UI::DrawVec2Control("Offset", component.camera.offset, 0.1f, 0.f, columnWidth);
                UI::DrawFloatControl("Zoom", &component.camera.zoom, 0.1f, 100.f, 0.01f, columnWidth);

                if (UI::DrawBoolControl("Is Primary?", &component.isPrimary, columnWidth))
                {
                    if (component.isPrimary && prevActiveCameraEntity != Entity_Null)
                    {
                        auto& prevCameraComponent = prevActiveCameraEntity.GetComponent<Camera2DComponent>();
                        prevCameraComponent.isPrimary = false;
                    }
                }

                UI::DrawColorControl("Clear Color", component.clearColor, columnWidth);
            };

            const auto DrawCamera3DComponent = [](Camera3DComponent& component)
            {
                const float columnWidth = 105.f;
                Entity prevActiveCameraEntity = Scenes::GetActiveCameraEntity3D();

                UI::DrawFloatControl("FOV", &component.camera.fov, 0.1f, 100.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Near Clip", &component.camera.nearClip, 0.01f, 10.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Far Clip", &component.camera.farClip, 1.f, 1000.f, 0.01f, columnWidth);

                if (UI::DrawBoolControl("Is Primary?", &component.isPrimary, columnWidth))
                {
                    if (component.isPrimary && prevActiveCameraEntity != Entity_Null)
                    {
                        auto& prevCameraComponent = prevActiveCameraEntity.GetComponent<Camera3DComponent>();
                        prevCameraComponent.isPrimary = false;
                    }
                }

                UI::DrawColorControl("Clear Color", component.clearColor, columnWidth);
            };

            const auto DrawRigidbody2DComponent = [](Rigidbody2DComponent& component)
            {
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

                UI::DrawFloatControl("Gravity Scale", &component.gravityScale, 0.f, 0.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Linear Damping", &component.linearDamping, 0.f, 0.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Angular Damping", &component.angularDamping, 0.f, 0.f, 0.01f, columnWidth);
            };

            const auto DrawBoxCollider2DComponent = [](BoxCollider2DComponent& component)
            {
                const float columnWidth = 100.f;
                UI::DrawVec2Control("Offset", component.offset, 0.1f, 0.f, columnWidth);
                UI::DrawVec2Control("Size", component.size, 0.1f, 0.f, columnWidth);
                UI::DrawBoolControl("Is Trigger?", &component.isTrigger, columnWidth);
                UI::DrawFloatControl("Density", &component.density, 0.f, 0.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Friction", &component.friction, 0.f, 1.f, 0.01f, columnWidth);
                UI::DrawFloatControl("Restitution", &component.restitution, 0.f, 1.f, 0.01f, columnWidth);
            };

            const auto DrawNativeScriptComponent = [](NativeScriptComponent& component)
            {
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
            };

            DrawComponent<TransformComponent>("Transform", entity, DrawTransformComponent);
            DrawComponent<DirectionalLightComponent>("Directional Light", entity, DrawDirectionalLightComponent);
            DrawComponent<CircleRendererComponent>("Circle Renderer", entity, DrawCircleRendererComponent);
            DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, DrawSpriteRendererComponent);
            DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, DrawMeshRendererComponent);
            DrawComponent<Animator2DComponent>("Animator 2D", entity, DrawAnimator2DComponent);
            DrawComponent<Camera2DComponent>("Orthographic Camera", entity, DrawCamera2DComponent);
            DrawComponent<Camera3DComponent>("Perspective Camera", entity, DrawCamera3DComponent);
            DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, DrawRigidbody2DComponent);
            DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, DrawBoxCollider2DComponent);
            DrawComponent<NativeScriptComponent>("Native Script", entity, DrawNativeScriptComponent);

            const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            const float availableWidth = ImGui::GetContentRegionAvail().x;
            const ImVec2 buttonSize = ImVec2(availableWidth * 0.8f, lineHeight);
            ImGui::Separator();
            ImGui::Indent(availableWidth * 0.5f - buttonSize.x * 0.5f);
            if (ImGui::Button("Add component", buttonSize))
                ImGui::OpenPopup("Add Component");

            if (ImGui::BeginPopup("Add Component"))
            {
                ImGui::SeparatorText("General");
                const bool shouldAddTransform = ImGui::MenuItem("Transform");
                const bool shouldAddNativeScript = ImGui::MenuItem("Native Script");

                ImGui::SeparatorText("2D");
                const bool shouldAddCircleRenderer = ImGui::MenuItem("Circle Renderer");
                const bool shouldAddSpriteRenderer = ImGui::MenuItem("Sprite Renderer");
                const bool shouldAddAnimator2D = ImGui::MenuItem("Animator 2D");
                const bool shouldAddCamera2D = ImGui::MenuItem("Camera 2D");
                const bool shouldAddRigidbody2D = ImGui::MenuItem("Rigidbody 2D");
                const bool shouldAddBoxCollider2D = ImGui::MenuItem("Box Collider 2D");

                ImGui::SeparatorText("3D");
                const bool shouldAddMeshRenderer = ImGui::MenuItem("Mesh Renderer");
                const bool shouldAddCamera3D = ImGui::MenuItem("Camera 3D");

                ImGui::SeparatorText("Lights");
                const bool shouldAddDirectionalLight = ImGui::MenuItem("Directional Light");

                if (shouldAddTransform)
                {
                    entity.AddComponent<TransformComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddNativeScript)
                {
                    entity.AddComponent<NativeScriptComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddCircleRenderer)
                {
                    entity.AddComponent<CircleRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddSpriteRenderer)
                {
                    entity.AddComponent<SpriteRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddAnimator2D)
                {
                    entity.AddComponent<Animator2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddCamera2D)
                {
                    entity.AddComponent<Camera2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddRigidbody2D)
                {
                    entity.AddComponent<Rigidbody2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddBoxCollider2D)
                {
                    entity.AddComponent<BoxCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddMeshRenderer)
                {
                    entity.AddComponent<MeshRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddCamera3D)
                {
                    entity.AddComponent<Camera3DComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if (shouldAddDirectionalLight)
                {
                    entity.AddComponent<DirectionalLightComponent>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

        template <typename T, typename UIFunction>
        void DrawComponent(const char* name, Entity entity, UIFunction callback)
        {
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap |
                                             ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;

            if (entity.HasComponent<T>())
            {
                ImGui::PushID(name);
                auto& component = entity.GetComponent<T>();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.f, 4.f));
                const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
                const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
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

                ImGui::PopID();

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
