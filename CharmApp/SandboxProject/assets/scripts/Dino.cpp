#include "Dino.h"

void Dino::OnCreate()
{
    ASSERT_ERROR(HasComponent<Animator2DComponent>(), "Dino::OnCreate - No Animator2D attached!");
    ASSERT_ERROR(HasComponent<Rigidbody2DComponent>(), "Dino::OnCreate - No Rigidbody2D attached!");
}

void Dino::OnUpdate()
{
    auto& animator = GetComponent<Animator2DComponent>();
    auto& body = GetComponent<Rigidbody2DComponent>();
    auto& transform = GetComponent<TransformComponent>();

    const float horizontalInput = Input::GetAxis(InputAxis::Horizontal);
    glm::vec2 movement;
    movement.x = horizontalInput * _speed;
    movement.y = 0.f;

    if (glm::abs(movement.x) > 0.f)
    {
        if (movement.x > 0.f)
        {
            if (_direction == k_directionLeft)
                transform.scale.x *= -1.f;

            _direction = k_directionRight;
        }

        if (movement.x < 0.f)
        {
            if (_direction == k_directionRight)
                transform.scale.x *= -1.f;

            _direction = k_directionLeft;
        }

        animator.SwitchToAnimation(k_animationRun);
    }
    else
        animator.SwitchToAnimation(k_animationIdle);

    body.AddForce(movement, ForceMode::Force);
}
