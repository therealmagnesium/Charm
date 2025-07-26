#include "TopDownController.h"

void TopDownController::OnCreate()
{
}

void TopDownController::OnUpdate()
{
    auto transform = TryGetComponent<TransformComponent>();
    auto rb2D = TryGetComponent<Rigidbody2DComponent>();

    if (rb2D == NULL)
        return;

    rb2D->linearVelocity.x = Input::GetInputAxis(InputAxis::Horizontal) * m_speed;
    rb2D->linearVelocity.y = Input::GetInputAxis(InputAxis::Vertical) * m_speed;
}
