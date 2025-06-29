#pragma once
#include <Charm.h>

using namespace Charm::ECS;

namespace Charm
{
    struct SceneHeirarchyState
    {
        Scene* context = NULL;
        Entity selectionContext;
        u32 textureSelectionIndex = 0;
    };

    namespace SceneHeirarchyPanel
    {
        void Display();
        void SetContext(Scene& context);
        void SetSelectedEntity(const Entity& entity);

        Scene* GetContext();
    }
}
