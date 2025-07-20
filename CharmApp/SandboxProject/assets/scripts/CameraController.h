#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

class CameraController : public Scriptable
{
public:
    void OnCreate() override;
    void OnUpdate() override;

private:
    Entity m_target;
};
