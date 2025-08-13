#pragma once
#include "Core/Base.h"
#include "Core/Asset.h"

namespace Charm
{
    namespace ECS
    {
        struct SpriteRendererComponent;
    }

    namespace Graphics
    {
        enum class SpriteSheetAnimType : u8
        {
            Horizontal = 0,
            Vertical,
        };

        struct Animation : public Core::Asset
        {
            u32 currentFrame = 0;
            u32 counter = 0;
            u32 speed = 0;
            u32 frameCount = 0;
            u32 rowCount = 0;
            u32 rowOffset = 0;
            u32 columnCount = 0;
            u32 columnOffset = 0;
            bool shouldLoop = false;
            bool hasFinished = false;
            SpriteSheetAnimType spriteSheetType = SpriteSheetAnimType::Horizontal;

            Core::AssetType GetType() const override { return Core::AssetType::Animation; }
        };

        /*
                struct AnimationController : Core::Asset
                {
                    std::vector<Core::AssetHandle> animations;
                    s32 activeSlot = -1;
                };*/

        inline const Animation Animation_Null;

        namespace Animations
        {
            Animation Load(const char* path);
            void Save(const char* path, const Animation& animation);
            void Reset(Animation& animation);
            void Update(Animation& animation, ECS::SpriteRendererComponent& spriteRenderer);
        }
    }
}
