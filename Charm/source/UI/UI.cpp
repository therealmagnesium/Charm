#include "UI/UI.h"
#include "Core/Log.h"
#include "Graphics/Window.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>
#include <SDL3/SDL_video.h>

using namespace Charm::Graphics;

namespace Charm
{
    namespace UI
    {
        void SetupContext()
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

            float fontSize = 16.f;
            ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/montserrat/Montserrat-Regular.ttf", fontSize);
            io.FontDefault = font;

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

            SDL_Window* windowHandle = (SDL_Window*)Window::GetHandle();
            SDL_GLContext windowContext = (SDL_GLContext)Window::GetContext();
            ImGui_ImplSDL3_InitForOpenGL(windowHandle, windowContext);
            ImGui_ImplOpenGL3_Init("#version 450");

            INFO("The UI context was setup successfully");
        }

        void DestroyContext()
        {
            INFO("The UI context is shutting down...");

            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplSDL3_Shutdown();
            ImGui::DestroyContext();
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
    }
}
