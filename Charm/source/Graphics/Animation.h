#pragma once
#include "Core/Base.h"
#include "Core/Asset.h"
#include "Graphics/Shapes.h"

#include <vector>

namespace Charm
{
    namespace Graphics
    {
        struct Texture;

        enum class SpriteSheetAnimType : u8
        {
            Horizontal = 0,
            Vertical,
            _TotalCount
        };

        struct Animation : public Core::Asset
        {
            u32 currentFrame = 0;
            u32 counter = 0;
            u32 speed = 0;
            u32 frameCount = 0;
            u32 rowOffset = 0;
            u32 columnOffset = 0;
            bool shouldLoop = false;
            bool hasFinished = false;
            SpriteSheetAnimType spriteSheetType = SpriteSheetAnimType::Horizontal;

            Core::AssetType GetType() const override { return Core::AssetType::Animation; }
        };

        struct AnimationController : Core::Asset
        {
            std::vector<Core::AssetHandle> animations;
            Core::AssetType GetType() const override { return Core::AssetType::AnimationController; }
        };

        inline const Animation Animation_Null;
        inline const AnimationController AnimationController_Null;

        namespace Animations
        {
            Animation Load(const char* path);
            void Save(const char* path, const Animation& animation);
            void Reset(Animation& animation);
            void Update(Animation& animation);
            void Apply(Animation& animation, Rectangle& rect, const Texture& texture);

            AnimationController LoadController(const char* path);
            void SaveController(const char* path, const AnimationController& controller);
        }
    }
}
