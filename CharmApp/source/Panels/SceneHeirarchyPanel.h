#pragma once
#include <ECS/Scene.h>
#include <ECS/Entity.h>

using namespace Charm::ECS;

namespace CharmApp
{
    struct SceneHeirarchyState
    {
        Scene* context = NULL;
        Entity selectionContext;
    };

    namespace SceneHeirarchyPanel
    {
        void Display();
        void SetContext(Scene& context);
        void SetSelectedEntity(const Entity& entity);

        Scene* GetContext();
        Entity& GetSelectedEntity();
    }
}
