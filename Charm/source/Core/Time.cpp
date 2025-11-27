#include "Core/Time.h"
#include <SDL3/SDL_timer.h>

namespace Charm
{
    namespace Core
    {
        static TimeState state;

        namespace Time
        {
            void Initialize(u32 targetFPS)
            {
                state.targetFramerate = targetFPS;
                state.currentTime = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
                state.lastTime = state.currentTime;
            }

            void Update()
            {
                const double targetFrameTime = 1.0 / (double)state.targetFramerate;

                // Use high-resolution timer
                state.currentTime = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
                state.deltaTime = state.currentTime - state.lastTime;

                // If we're running faster than target, delay
                if (state.deltaTime < targetFrameTime)
                {
                    double delayTime = targetFrameTime - state.deltaTime;

                    // For very short delays, use SDL_DelayNS for nanosecond precision
                    if (delayTime > 0.001) // If delay > 1ms, use regular SDL_Delay
                    {
                        SDL_Delay((u32)(delayTime * 1000.0));
                    }
                    else // For sub-millisecond delays, use nanosecond delay
                    {
                        SDL_DelayNS((u64)(delayTime * 1000000000.0));
                    }

                    // Update time after delay
                    state.currentTime = (double)SDL_GetPerformanceCounter() / (double)SDL_GetPerformanceFrequency();
                    state.deltaTime = state.currentTime - state.lastTime;
                }

                state.lastTime = state.currentTime;
            }

            void StartTimer(Timer& timer, double target)
            {
                ResetTimer(timer, target);
                UnpauseTimer(timer);
            }

            void ResetTimer(Timer& timer, double target)
            {
                timer.isDone = false;
                timer.isPaused = true;
                timer.elapsed = 0.0;
                timer.target = target;
            }

            void UpdateTimer(Timer& timer)
            {
                if (!timer.isPaused && !timer.isDone)
                {
                    timer.elapsed += GetDelta();

                    if (timer.elapsed >= timer.target && timer.target != 0.0)
                    {
                        timer.isPaused = true;
                        timer.isDone = true;
                    }
                }
            }

            void PauseTimer(Timer& timer) { timer.isPaused = true; }
            void UnpauseTimer(Timer& timer) { timer.isPaused = false; }

            double GetCurrent() { return state.currentTime; }
            double GetDelta() { return state.deltaTime; }
        }
    }
}
