#pragma once
#include "Core/Base.h"

namespace Charm
{
    namespace Core
    {
        struct TimeState
        {
            u32 targetFramerate = 0;
            double currentTime = 0.0;
            double lastTime = 0.0;
            double deltaTime = 0.0;
        };

        namespace Time
        {
            void Initialize(u32 targetFPS);
            void Update();

            double GetCurrent();
            double GetDelta();
        }
    }
}
