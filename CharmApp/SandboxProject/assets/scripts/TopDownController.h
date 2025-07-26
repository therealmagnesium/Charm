#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

class TopDownController : public Scriptable
{
public:
    void OnCreate() override;
    void OnUpdate() override;

private:
    float m_speed = 2.f;
};
