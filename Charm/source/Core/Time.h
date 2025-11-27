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

        struct Timer
        {
            bool isDone = false;
            bool isPaused = true;
            double elapsed = 0.0;
            double target = 0.0;
        };

        namespace Time
        {
            void Initialize(u32 targetFPS);
            void Update();

            void StartTimer(Timer& timer, double target = 0.0);
            void ResetTimer(Timer& timer, double target = 0.0);
            void UpdateTimer(Timer& timer);
            void PauseTimer(Timer& timer);
            void UnpauseTimer(Timer& timer);

            double GetCurrent();
            double GetDelta();

        }
    }
}
