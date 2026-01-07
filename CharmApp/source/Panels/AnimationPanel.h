#pragma once
#include <Core/Base.h>
#include <Core/Asset.h>

using namespace Charm;

namespace CharmApp
{
    struct AnimationPanelState
    {
        Core::AssetHandle spriteSheet = Core::AssetHandle_Invalid;
        Core::AssetHandle selectedAnimation = Core::AssetHandle_Invalid;
        bool shouldDisplay = false;
    };

    namespace AnimationPanel
    {
        void Display();
        void Toggle();

        bool ShouldDisplay();
    }
}
