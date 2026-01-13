#pragma once
#include "Core/Base.h"
#include <random>

namespace Charm
{
    namespace Core
    {
        using UUID = u64;

        struct RandomState
        {
            std::random_device device;
            std::mt19937_64 engine;
            std::uniform_int_distribution<UUID> uuidDistribution;
        };

        namespace Random
        {
            void Init();
            UUID GenerateUUID();
            u32 Generate(u32 min, u32 max);
            float Generate(float min, float max);
        }

        inline const UUID UUID_Invalid = 0;
    }
}
