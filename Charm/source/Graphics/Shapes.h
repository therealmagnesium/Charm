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

        struct Rectangle
        {
            float x = 0.f;
            float y = 0.f;
            float width = 0.f;
            float height = 0.f;
        };
    }
}
