#include "UI/UI.h"

#include "Core/AssetManager.h"
#include "Core/FileDialogs.h"
#include "Core/Log.h"
#include "Core/Utils.h"

#include "Graphics/Animation.h"
#include "Graphics/Texture.h"
#include "Graphics/TilePalette.h"
#include "Graphics/Window.h"

#include "Projects/Project.h"
#include "Projects/ProjectSerializer.h"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <ImGuizmo.h>
#include <SDL3/SDL_video.h>
#include <glm/gtc/type_ptr.hpp>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::Projects;

namespace Charm
{
    namespace UI
    {
        static bool isContextValid = false;

        void SetupTheme_MyPurple();
        void SetupTheme_ComfyDarkCyan();

        void SetupContext()
        {
            if (isContextValid)
            {
                WARN("Cannot setup the UI context more than once");
                return;
            }

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            SetupTheme_ComfyDarkCyan();

            const float fontSize = 18.f;
            ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/montserrat/Montserrat-Regular.ttf", fontSize);
            io.FontDefault = font;

            SDL_Window* windowHandle = (SDL_Window*)Window::GetHandle();
            SDL_GLContext windowContext = (SDL_GLContext)Window::GetContext();

            const bool isSDL3BackendValid = ImGui_ImplSDL3_InitForOpenGL(windowHandle, windowContext);
            const bool isOpenGLBackendValid = ImGui_ImplOpenGL3_Init("#version 450");

            ASSERT(isSDL3BackendValid == true, "Failed to setup SDL backend for ImGui!");
            ASSERT(isOpenGLBackendValid == true, "Failed to setup OpenGL backend for ImGui!");

            isContextValid = true;
            INFO("The UI context was setup successfully");
        }

        void DestroyContext()
        {
            if (!isContextValid)
            {
                WARN("Cannot destory the UI context because it hasn't been setup");
                return;
            }

            INFO("The UI context is shutting down...");

            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();

            isContextValid = false;
        }

        void HandleEvents(void* event)
        {
            ImGui_ImplSDL3_ProcessEvent((SDL_Event*)event);
        }

        void BeginFrome()
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            ImGuizmo::BeginFrame();
        }

        void EndFrame()
        {
            ImGui::Render();
        }

        void Display()
        {
            ImDrawData* drawData = ImGui::GetDrawData();
            ASSERT(drawData != NULL, "%s", "The UI crashed because there is no draw data!");

            ImGui_ImplOpenGL3_RenderDrawData(drawData);

            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }
        }

        bool DrawBoolControl(const char* label, bool* b, float columnWidth)
        {
            ImGui::PushID(label);
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            bool hasChanged = ImGui::Checkbox("##", b);
            ImGui::Columns(1);
            ImGui::PopID();

            return hasChanged;
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

            ImGui::PushMultiItemsWidths(2, ImGui::GetContentRegionAvail().x - buttonSize.x * 3.f);
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, ImGui::GetFontSize() / 2.f));

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
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, ImGui::GetFontSize() / 2.f));

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

        void DrawColorControl(const char* label, glm::vec4& v, float columnWidth)
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

        void DrawIntInputControl(const char* label, s32* v, s32 min, s32 max, float columnWidth)
        {
            ImGui::PushID(label);
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.f);

            if (ImGui::InputInt("##", v))
            {
                if (*v < min)
                    *v = min;

                if (*v >= max && min < max)
                    *v = max;
            }

            ImGui::Columns(1);
            ImGui::PopID();
        }

        void DrawTextInputControl(const char* label, std::string* s, u32 flags, float columnWidth)
        {
            ImGui::PushID(label);
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.f);
            ImGui::InputText("##", s, flags);
            ImGui::Columns(1);
            ImGui::PopID();
        }

        bool DrawFilesystemInputControl(const char* label, std::filesystem::path* p, u32 flags, float columnWidth)
        {
            if (p == NULL)
                return false;

            std::string stringPath = p->string();

            ImGui::PushID(label);
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("%s", label);
            ImGui::NextColumn();

            const ImVec2 availRegion = ImGui::GetContentRegionAvail();
            ImGui::SetNextItemAllowOverlap();
            if (ImGui::InputText("##", &stringPath, flags))
                *p = stringPath;

            const float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.f;
            const ImVec2 buttonSize = ImVec2(150, lineHeight);
            ImGui::SameLine();

            bool showSaveDialog = ImGui::Button("Select Path", buttonSize);

            ImGui::Columns(1);
            ImGui::PopID();

            return showSaveDialog;
        }

        void DrawAssetControls_Animation(Animation* animation)
        {
            const float columnWidth = 150.f;
            const std::filesystem::path path = AssetManager::GetAssetPath(animation->handle);
            const std::string stringPath = path.string();
            const std::string fileName = path.filename().string();
            const char* name = fileName.c_str();

            ImGui::PushID("Name");
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("Name");
            ImGui::NextColumn();
            ImGui::Text("%s", name);
            ImGui::Columns(1);
            ImGui::PopID();

            ImGui::PushID("Sprite Sheet Type");
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::Text("Sprite Sheet Type");
            ImGui::NextColumn();

            std::string typeAsString = Utils::SpriteSheetAnimTypeToString(animation->spriteSheetType);
            std::string placeholder = typeAsString;
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::BeginCombo("##", placeholder.c_str()))
            {
                const char* types[(u32)SpriteSheetAnimType::_TotalCount] = {"Horizontal", "Vertical"};

                for (u8 i = 0; i < LEN(types); i++)
                {
                    bool isSelected = typeAsString == types[i];

                    if (ImGui::Selectable(types[i], isSelected))
                        animation->spriteSheetType = (SpriteSheetAnimType)i;
                }
                ImGui::EndCombo();
            }

            ImGui::Columns(1);
            ImGui::PopID();

            DrawIntInputControl("Speed", (s32*)&animation->speed, 0, 0, columnWidth);
            DrawIntInputControl("Frame Count", (s32*)&animation->frameCount, 0, 0, columnWidth);
            DrawIntInputControl("Row Offset", (s32*)&animation->rowOffset, 0, 0, columnWidth);
            DrawIntInputControl("Column Offset", (s32*)&animation->columnOffset, 0, 0, columnWidth);
            DrawBoolControl("Should Loop?", &animation->shouldLoop, columnWidth);

            // DrawIntInputControl("Row Count", (s32*)&animation->rowCount, 0, 0, columnWidth);
            // DrawIntInputControl("Column Count", (s32*)&animation->columnCount, 0, 0, columnWidth);

            if (ImGui::Button("Save"))
                Animations::Save(stringPath.c_str(), *animation);
        }

        void DrawAssetControls_AnimationController(AnimationController* controller)
        {
            if (ImGui::Button("Add Animation"))
            {
                FileDialogFilter filter;
                filter.name = "Animation";
                filter.specification = "anim";

                if (FileDialogs::Open(&filter, 1))
                {
                    const Project& project = ProjectSerializer::GetContext();
                    const std::filesystem::path path = FileDialogs::GetSelectedPath();
                    const std::filesystem::path relativePath = std::filesystem::relative(path, ProjectManager::GetAssetPath(project));
                    const std::filesystem::path projectPath = ProjectManager::GetAssetFileSystemPath(relativePath, project);

                    AssetHandle handle = AssetManager::FindAssetHandle(projectPath.string());
                    if (AssetManager::IsHandleValid(handle))
                        controller->animations.emplace_back(handle);
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Save"))
            {
                // const std::filesystem::path& path = ContentBrowserPanel::GetSelectedFilePath();
                const std::filesystem::path path = AssetManager::GetAssetPath(controller->handle);
                Animations::SaveController(path.string().c_str(), *controller);
            }

            const ImGuiTableFlags flags =
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_Reorderable |
                //  ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable |
                ImGuiTableFlags_SortMulti |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_NoBordersInBody;
            // ImGuiTableFlags_SizingFixedFit |
            // ImGuiTableFlags_ScrollX |
            // ImGuiTableFlags_ScrollY;

            if (ImGui::BeginTable("Animations", 4, flags))
            {
                ImGui::TableSetupColumn("Slot", 0, 0.f, 0);
                ImGui::TableSetupColumn("Animation", 0, 0.f, 1);
                ImGui::TableSetupColumn("Reorder", 0, 0.f, 2);
                ImGui::TableSetupColumn("Remove", 0, 0.f, 3);
                ImGui::TableHeadersRow();

                for (u32 i = 0; i < controller->animations.size(); i++)
                {
                    AssetHandle animationHandle = controller->animations[i];
                    Animation* animation = AssetManager::GetAsset<Animation>(animationHandle);
                    const std::filesystem::path path = AssetManager::GetAssetPath(animation->handle);
                    const std::string stemName = path.stem().string();

                    ImGui::PushID(animation->handle + i);
                    ImGui::TableNextRow();

                    if (ImGui::TableSetColumnIndex(0))
                        ImGui::Text("%d", i);

                    if (ImGui::TableSetColumnIndex(1))
                        ImGui::Text("%s", stemName.c_str());

                    if (ImGui::TableSetColumnIndex(2))
                    {
                        if (ImGui::SmallButton("Move Up") && i != 0)
                            std::swap(controller->animations[i], controller->animations[i - 1]);

                        if (ImGui::SmallButton("Move Down") && i != controller->animations.size() - 1)
                            std::swap(controller->animations[i], controller->animations[i + 1]);
                    }

                    if (ImGui::TableSetColumnIndex(3))
                        if (ImGui::Button("Remove"))
                            controller->animations.erase(controller->animations.begin() + i);

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }
        }

        void DrawAssetControls_Texture(Texture* texture)
        {
            if (texture == NULL) return;

            const float columnWidth = 130.f;
            const char* filters[(u32)TextureFilter::_TotalCount] = {"Linear", "Nearest",
                                                                    "Linear Mipmap Linear", "Linear Mipmap Nearest",
                                                                    "Nearest Mipmap Linear", "Nearest Mipmap Nearest"};
            const char* modes[(u32)TextureMode::_TotalCount] = {"Single", "Sprite Sheet", "Tileset"};

            ImGui::PushID(texture->handle);

            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);

            ImGui::Text("Name");
            ImGui::Text("Size (pixels)");
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Min Filter");
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Mag Filter");
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Mode");
            ImGui::NextColumn();

            const std::string name = AssetManager::GetAssetPath(texture->handle).stem().string();
            ImGui::Text("%s", name.c_str());
            ImGui::Text("%dx%d", texture->width, texture->height);

            const std::string minFilterAsString = Utils::TextureFilterToString(texture->minFilter);
            std::string placeholder = minFilterAsString;
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::BeginCombo("##Texture Min Filter", placeholder.c_str()))
            {
                for (u8 i = 0; i < LEN(filters); i++)
                {
                    const bool isSelected = minFilterAsString == filters[i];

                    if (ImGui::Selectable(filters[i], isSelected))
                        texture->minFilter = (TextureFilter)i;
                }
                ImGui::EndCombo();
            }

            const std::string magFilterAsString = Utils::TextureFilterToString(texture->magFilter);
            placeholder = magFilterAsString;
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::BeginCombo("##Texture Mag Filter", placeholder.c_str()))
            {
                for (u8 i = 0; i < LEN(filters); i++)
                {
                    const bool isSelected = magFilterAsString == filters[i];

                    if (ImGui::Selectable(filters[i], isSelected))
                        texture->magFilter = (TextureFilter)i;
                }
                ImGui::EndCombo();
            }

            const std::string modeAsString = Utils::TextureModeToString(texture->mode);
            placeholder = modeAsString;
            ImGui::SetNextItemWidth(-1.f);
            if (ImGui::BeginCombo("##Texture Mode", placeholder.c_str()))
            {
                for (u8 i = 0; i < LEN(modes); i++)
                {
                    const bool isSelected = modeAsString == modes[i];

                    if (ImGui::Selectable(modes[i], isSelected))
                        texture->mode = (TextureMode)i;
                }
                ImGui::EndCombo();
            }

            ImGui::Columns(1);

            DrawIntInputControl("Pixels Per Unit", (s32*)&texture->pixelsPerUnit, 0, 0, columnWidth);

            const float targetAspect = (float)texture->width / (float)texture->height;
            ImVec2 regionSize = ImGui::GetContentRegionAvail();
            ImVec2 aspectSize = ImVec2(regionSize.x, regionSize.x / targetAspect);

            if (aspectSize.y > regionSize.y)
            {
                aspectSize.y = regionSize.y;
                aspectSize.x = regionSize.y * targetAspect;
            }

            ImGui::Image(texture->id, aspectSize, ImVec2(0.f, 0.f), ImVec2(1.f, 1.f));

            if (ImGui::Button("Apply"))
                Textures::Invalidate(*texture);

            ImGui::PopID();
        }

        void DrawAssetControls_TilePalette(Graphics::TilePalette* tilePalette)
        {
            if (tilePalette == NULL)
                return;

            ImGui::PushID(tilePalette->handle);

            const float columnWidth = 130.f;
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, columnWidth);
            ImGui::TextUnformatted("Name");
            ImGui::TextUnformatted("Slice Width");
            ImGui::TextUnformatted("Slice Height");
            ImGui::TextUnformatted("Total Tile Count");
            ImGui::NextColumn();

            const std::string name = AssetManager::GetAssetPath(tilePalette->handle).stem().string();
            ImGui::Text("%s", name.c_str());
            ImGui::Text("%d", tilePalette->sliceWidth);
            ImGui::Text("%d", tilePalette->sliceHeight);
            ImGui::Text("%d", tilePalette->totalTileCount);
            ImGui::Columns(1);

            ImGui::Separator();
            ImGui::TextUnformatted("Open the \"Tile Palette\" window to paint with a palette");

            ImGui::PopID();
        }

        void SetupTheme_MyPurple()
        {
            ImGuiStyle& style = ImGui::GetStyle();
            style.WindowMinSize.x = 380.f;

            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.075f, 0.08f, 0.102f, 1.f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.085f, 0.08f, 0.09f, 1.f);

            style.Colors[ImGuiCol_Header] = ImVec4(0.112f, 0.112f, 0.236f, 1.f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.212f, 0.212f, 0.336f, 1.f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.012f, 0.012f, 0.136f, 1.f);

            style.Colors[ImGuiCol_Button] = ImVec4(0.112f, 0.112f, 0.236f, 1.f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.212f, 0.212f, 0.336f, 1.f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.012f, 0.012f, 0.136f, 1.f);

            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.212f, 0.212f, 0.336f, 1.f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.312f, 0.312f, 0.436f, 1.f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.112f, 0.112f, 0.236f, 1.f);

            style.Colors[ImGuiCol_Tab] = ImVec4(0.162f, 0.152f, 0.286f, 1.f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.212f, 0.212f, 0.336f, 1.f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.162f, 0.362f, 0.386f, 1.f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.162f, 0.162f, 0.286f, 1.f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.162f, 0.162f, 0.286f, 1.f);

            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.112f, 0.112f, 0.236f, 1.f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.112f, 0.112f, 0.236f, 1.f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.112f, 0.112f, 0.236f, 1.f);

            style.Colors[ImGuiCol_Border] = ImVec4(0.112f, 0.112f, 0.176f, 0.502f);

            style.Colors[ImGuiCol_Separator] = ImVec4(0.162f, 0.362f, 0.386f, 0.5f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.262f, 0.462f, 0.486f, 0.5f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.062f, 0.262f, 0.286f, 0.5f);
        }

        void SetupTheme_ComfyDarkCyan()
        {
            // Comfortable Dark Cyan style by SouthCraftX from ImThemes
            ImGuiStyle& style = ImGui::GetStyle();

            style.Alpha = 1.0f;
            style.DisabledAlpha = 1.0f;
            style.WindowPadding = ImVec2(10.0f, 20.0f);
            style.WindowRounding = 11.5f;
            style.WindowBorderSize = 0.0f;
            style.WindowMinSize = ImVec2(20.0f, 20.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Right;
            style.ChildRounding = 20.0f;
            style.ChildBorderSize = 1.0f;
            style.PopupRounding = 17.39999961853027f;
            style.PopupBorderSize = 1.0f;
            // style.FramePadding = ImVec2(20.0f, 3.400000095367432f);
            style.FrameRounding = 11.89999961853027f;
            style.FrameBorderSize = 0.0f;
            style.ItemSpacing = ImVec2(8.899999618530273f, 10.5f);
            style.ItemInnerSpacing = ImVec2(7.099999904632568f, 1.799999952316284f);
            style.CellPadding = ImVec2(12.10000038146973f, 9.199999809265137f);
            // style.IndentSpacing = 0.0f;
            style.ColumnsMinSpacing = 8.699999809265137f;
            style.ScrollbarSize = 11.60000038146973f;
            style.ScrollbarRounding = 15.89999961853027f;
            style.GrabMinSize = 3.700000047683716f;
            style.GrabRounding = 20.0f;
            style.TabRounding = 9.800000190734863f;
            style.TabBorderSize = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

            style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09411764889955521f, 0.1019607856869698f, 0.1176470592617989f, 1.0f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.1137254908680916f, 0.125490203499794f, 0.1529411822557449f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.6000000238418579f, 0.9647058844566345f, 0.0313725508749485f, 1.0f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.1411764770746231f, 0.1647058874368668f, 0.2078431397676468f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2103921610116959f, 0.2182352977991104f, 0.276078434586525f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1829411822557449f, 0.1829411822557449f, 0.2429411822557449f, 1.0f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.1411764770746231f, 0.1647058874368668f, 0.2078431397676468f, 1.0f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2011764770746231f, 0.2247058874368668f, 0.2878431397676468f, 1.0f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.1294117718935013f, 0.1490196138620377f, 0.1921568661928177f, 1.0f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.125490203499794f, 0.2745098173618317f, 0.572549045085907f, 1.0f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.4372549057006836f, 0.4372549057006836f, 0.4372549057006836f, 1.0f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2666666805744171f, 0.2901960909366608f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
        }
    }
}
