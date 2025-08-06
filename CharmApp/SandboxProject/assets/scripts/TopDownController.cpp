#include "TopDownController.h"

void TopDownController::OnCreate()
{
    ASSERT_ERROR(HasComponent<Rigidbody2DComponent>(), "TopDownController::OnCreate - No Rigidbody2D attached!");
}

void TopDownController::OnUpdate()
{
    auto& transform = GetComponent<TransformComponent>();
    auto& body = GetComponent<Rigidbody2DComponent>();

    glm::vec2 movement;
    movement.x = Input::GetAxis(InputAxis::Horizontal) * _speed;
    movement.y = Input::GetAxis(InputAxis::Vertical) * _speed;

    body.AddForce(movement, ForceMode::Impulse);
}
