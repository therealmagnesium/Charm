#pragma once
#include <Charm.h>

using namespace Charm;
using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

class Dino : public Scriptable
{
public:
    void OnCreate();
    void OnUpdate();

private:
    const u32 k_animationIdle = 0;
    const u32 k_animationRun = 1;
    const float k_directionRight = 1.f;
    const float k_directionLeft = -1.f;

    float _speed = 25.f;
    float _direction = k_directionRight;
};
