#pragma once
#include <Core/Base.h>
#include <Core/Asset.h>
#include <Graphics/Shapes.h>
#include <Graphics/Texture.h>
#include <ECS/Entity.h>

using namespace Charm;

namespace CharmApp
{
    enum class AnimationIcon : u8
    {
        StepForward = 0,
        StepBackward,
        StepFront,
        StepBack,
        Count
    };

    struct AnimationPanelState
    {
        Graphics::Texture icons[(u32)AnimationIcon::Count];
        Core::AssetHandle spriteSheet = Core::AssetHandle_Invalid;
        Core::AssetHandle selectedAnimation = Core::AssetHandle_Invalid;
        Graphics::Rectangle* activeCrop = NULL;
        ECS::Entity trackedEntity = ECS::Entity_Null;
        bool shouldPreviewAnimation = false;
        bool shouldDisplay = true;
    };

    namespace AnimationPanel
    {
        void Init();
        void Shutdown();
        void Display();
        void Toggle();

        bool ShouldDisplay();
    }
}
