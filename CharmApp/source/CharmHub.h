#pragma once
#include <Charm.h>

using namespace Charm;
using namespace Charm::Projects;

namespace CharmHub
{
    struct CharmHubState
    {
        Project project;
        bool isProjectSelected = false;
        bool shouldDisplayNewProject = false;
        std::filesystem::path newProjectPath;
    };

    void OnCreate();
    void OnUpdate();
    void OnRender();
    void OnRenderUI();
    void OnShutdown();

    bool IsProjectSelected();
    const Project& GetProject();
}
