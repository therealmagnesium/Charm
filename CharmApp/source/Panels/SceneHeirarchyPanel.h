#pragma once
#include <Charm.h>

using namespace Charm::ECS;

namespace Charm
{
    struct SceneHeirarchyState
    {
        Scene* context = NULL;
        Entity selectionContext;
    };

    namespace SceneHeirarchyPanel
    {
        void Display();
        void SetContext(Scene* context);
    }
}
