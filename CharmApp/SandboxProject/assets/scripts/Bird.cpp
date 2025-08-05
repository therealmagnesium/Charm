#include "Bird.h"

void Bird::OnCreate()
{
    ASSERT_ERROR(HasComponent<Rigidbody2DComponent>(), "Bird does not have a Rigidbody2D component attached!");
}

void Bird::OnUpdate()
{
    const auto& body = GetComponent<Rigidbody2DComponent>();

    if (Input::IsKeyPressed(KEY_SPACE) && !_isDead)
        Jump();
}

void Bird::OnCollisionEnter(Entity& other)
{
    auto& otherInternal = other.GetComponent<InternalComponent>();
    if (otherInternal.tag == "Ground" || otherInternal.tag == "Pipe")
        _isDead = true;
}

void Bird::OnCollisionExit(Entity& other)
{
}

void Bird::Jump()
{
    const glm::vec2 up = glm::vec2(0.f, 1.f);
    const glm::vec2 force = up * _jumpForce;
    auto& body = GetComponent<Rigidbody2DComponent>();

    body.AddForce(force, ForceMode::Impulse);
}
