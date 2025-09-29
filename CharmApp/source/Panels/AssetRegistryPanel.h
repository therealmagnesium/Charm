#pragma once
#include "Core/Base.h"

namespace CharmApp
{
    namespace AssetRegistryPanel
    {
        struct AssetRegistryState
        {
            u32 flags = 0;
        };

        void Init();
        void Display();
    }
}
