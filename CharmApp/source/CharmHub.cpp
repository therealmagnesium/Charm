#include "CharmHub.h"
#include <Charm.h>
#include <imgui.h>
#include <imgui_stdlib.h>

using namespace Charm::Core;
using namespace Charm::Graphics;

namespace CharmHub
{
    static CharmHubState state;

    void OnCreate()
    {
        state.project = ProjectManager::New();
        FileDialogs::SetDefaultPath(std::filesystem::current_path());
    }

    void OnUpdate()
    {
        if (!state.project.startScenePath.empty() && state.isProjectSelected)
            Application::Quit();
    }

    void OnRender() {}

    void OnRenderUI()
    {
        ImGui::DockSpaceOverViewport();

        u32 flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::Begin("Projects", NULL, flags);

        ImGui::PushFontSize(48.f);
        const ImVec2 windowSize = ImGui::GetWindowSize();
        const std::string text = "Welcome to Charm!";
        const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
        const ImVec2 textPosition = ImVec2(windowSize.x / 2.f - textSize.x / 2.f, windowSize.y / 2.5f - textSize.y / 2.f);
        ImGui::SetCursorPos(textPosition);
        ImGui::TextUnformatted(text.c_str());
        ImGui::PopFontSize();

        ImGui::PushFontSize(24.f);
        const ImVec2 buttonSize = ImVec2(175, 50);
        const ImVec2 buttonPosition = ImVec2(windowSize.x / 2.f - buttonSize.x, windowSize.y / 2.f - buttonSize.y / 2.f);
        ImGui::SetCursorPos(buttonPosition);

        if (ImGui::Button("New Project", buttonSize))
            state.shouldDisplayNewProject = true;

        ImGui::SameLine();

        if (ImGui::Button("Load Project", buttonSize))
        {
            if (FileDialogs::Open())
            {
                const std::filesystem::path path = FileDialogs::GetSelectedPath();
                const std::filesystem::path relativePath = std::filesystem::relative(path, std::filesystem::current_path());
                state.project = ProjectManager::Load(relativePath.c_str());
                state.isProjectSelected = true;
            }
        }
        ImGui::PopFontSize();
        ImGui::End();

        if (state.shouldDisplayNewProject)
        {
            ImGui::Begin("New Project");
            ImGui::PushFontSize(20.f);

            if (ImGui::Button("Cancel", ImVec2(100.f, 25.f)))
                state.shouldDisplayNewProject = false;

            UI::DrawTextInputControl("Name", &state.project.name);
            if (UI::DrawFilesystemInputControl("Path", &state.newProjectPath, ImGuiInputTextFlags_ElideLeft))
            {
                if (FileDialogs::Save())
                {
                    state.newProjectPath = std::filesystem::relative(FileDialogs::GetSelectedPath(), std::filesystem::current_path());
                    state.project.name = state.newProjectPath.stem();
                }
            }

            ImGui::SetCursorPosY(windowSize.y - 60.f);
            if (ImGui::Button("Open New Project", ImVec2(200.f, 50.f)) && !state.newProjectPath.empty() && !state.project.name.empty())
            {
                ProjectManager::Save(state.newProjectPath.c_str(), state.project);
                state.project.startScenePath = "Scenes/SampleScene.charm";
                state.isProjectSelected = true;

                const std::filesystem::path sampleSceneRoot = ProjectManager::GetAssetFileSystemPath(state.project.startScenePath.parent_path(), state.project);
                if (!std::filesystem::exists(sampleSceneRoot))
                    std::filesystem::create_directory(sampleSceneRoot);

                const std::filesystem::path startScenePath = ProjectManager::GetAssetFileSystemPath(state.project.startScenePath, state.project);
                Scene sampleScene = Scenes::Create();
                SceneSerializer::SetContext(&sampleScene);
                SceneSerializer::Serialize(startScenePath.c_str());
                SceneSerializer::SetContext(NULL);
            }

            ImGui::PopFontSize();
            ImGui::End();
        }
    }
    void OnShutdown() {}

    bool IsProjectSelected() { return state.isProjectSelected; }
    const Project& GetProject() { return state.project; }

}
