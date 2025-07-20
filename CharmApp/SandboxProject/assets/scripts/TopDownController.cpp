#include "TopDownController.h"

void TopDownController::OnCreate()
{
}

void TopDownController::OnUpdate()
{
    auto& transform = GetComponent<TransformComponent>();

    if (Input::IsKeyDown(KEY_LEFT))
        transform.position.x -= m_speed * Time::GetDelta();

    if (Input::IsKeyDown(KEY_RIGHT))
        transform.position.x += m_speed * Time::GetDelta();

    if (Input::IsKeyDown(KEY_DOWN))
        transform.position.y -= m_speed * Time::GetDelta();

    if (Input::IsKeyDown(KEY_UP))
        transform.position.y += m_speed * Time::GetDelta();
}
