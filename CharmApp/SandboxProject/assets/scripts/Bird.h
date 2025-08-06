#pragma once
#include <Charm.h>

using namespace Charm::Core;
using namespace Charm::Graphics;
using namespace Charm::ECS;

class Bird : public Scriptable
{
public:
    void OnCreate() override;
    void OnUpdate() override;
    void OnCollisionEnter(Entity& other) override;
    void OnCollisionExit(Entity& other) override;

private:
    void Jump();

private:
    float _jumpForce = 3.f;
    bool _isDead = false;
    Entity _testEntity;
};
