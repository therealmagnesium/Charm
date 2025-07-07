#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace Graphics
    {
        enum class BodyType : u8
        {
            Static = 0,
            Dynamic,
            Kinematic,
        };

        enum class OriginMode : u8
        {
            Center = 0,
            Left,
            Right,
            BottomLeft,
            BottomMiddle,
            BottomRight,
            TopLeft,
            TopMiddle,
            TopRight
        };

        struct Rectangle
        {
            float x = 0.f;
            float y = 0.f;
            float width = 0.f;
            float height = 0.f;
        };
    }
}
