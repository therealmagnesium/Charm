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
            }

            void Update()
            {
                state.currentTime = (double)SDL_GetTicks() / 1000.0;
                state.deltaTime = state.currentTime - state.lastTime;
                state.lastTime = (double)SDL_GetTicks() / 1000.0;

                if (state.deltaTime < 1000.0 / state.targetFramerate)
                    SDL_Delay(1000.0 / state.targetFramerate - state.deltaTime);
            }

            double GetCurrent() { return state.currentTime; }
            double GetDelta() { return state.deltaTime; }
        }
    }
}
