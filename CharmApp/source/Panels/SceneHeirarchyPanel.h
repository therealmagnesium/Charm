#pragma once
#include <ECS/Scene.h>
#include <ECS/Entity.h>

using namespace Charm;

namespace CharmApp
{
    struct SceneHeirarchyState
    {
        ECS::Scene* context = NULL;
        ECS::Entity selectionContext;
        bool shouldDisplay = true;
    };

    namespace SceneHeirarchyPanel
    {
        void Display();
        void Toggle();
        void SetContext(ECS::Scene& context);
        void SetSelectedEntity(const ECS::Entity& entity);

        ECS::Scene* GetContext();
        ECS::Entity& GetSelectedEntity();
        bool ShouldDisplay();
    }
}
