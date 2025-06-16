#include "Core/Random.h"
#include "Core/Log.h"

namespace Charm
{
    namespace Core
    {
        static RandomState state;
        static bool isInitialized = false;

        namespace Random
        {
            void Init()
            {
                if (isInitialized)
                {
                    WARN("The random number generator cannot be initalized more than once");
                    return;
                }

                state.engine = std::mt19937_64(state.device());
                state.uuidDistribution = std::uniform_int_distribution<UUID>(0, std::numeric_limits<UUID>::max());
                isInitialized = true;
            }

            UUID GenerateUUID()
            {
                ASSERT(isInitialized, "Random::GenerateUUID - Random::Init must be called first");
                return state.uuidDistribution(state.engine);
            }

            u32 Generate(u32 min, u32 max)
            {
                std::uniform_int_distribution<u32> distribution(min, max);
                return distribution(state.engine);
            }

            float Generate(float min, float max)
            {
                std::uniform_real_distribution<float> distribution(min, max);
                return distribution(state.engine);
            }
        }
    }
}
