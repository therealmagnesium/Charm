#include "CameraController.h"

void CameraController::OnCreate()
{
    m_target = FindEntityWithTag("Yellow Square");
}

void CameraController::OnUpdate()
{
    auto& transform = GetComponent<TransformComponent>();
    auto& targetTransform = m_target.GetComponent<TransformComponent>();

    transform.position = targetTransform.position;
}
