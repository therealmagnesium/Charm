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
        void SetupTheme_MyPurple();
        void SetupTheme_ComfyDarkCyan();

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

            float fontSize = 18.f;
            ImFont* font = io.Fonts->AddFontFromFileTTF("assets/fonts/montserrat/Montserrat-Regular.ttf", fontSize);
            io.FontDefault = font;

            SetupTheme_ComfyDarkCyan();

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
            style.WindowMenuButtonPosition = ImGuiDir_None;
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
            style.Colors[ImGuiCol_Button] = ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1803921610116959f, 0.1882352977991104f, 0.196078434586525f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1529411822557449f, 0.1529411822557449f, 0.1529411822557449f, 1.0f);
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
