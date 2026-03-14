#include "ECS/Components.h"
#include <box2d/box2d.h>

namespace Charm::ECS
{
    void Rigidbody2DComponent::AddForce(const glm::vec2& force, ForceMode mode)
    {
        b2BodyId b2Body = *(b2BodyId*)&runtimeBody;
        b2Vec2 b2Force = (b2Vec2){force.x, force.y};

        switch (mode)
        {
            case ForceMode::Force:
                b2Body_ApplyForceToCenter(b2Body, b2Force, true);
                break;

            case ForceMode::Impulse:
                b2Body_ApplyLinearImpulseToCenter(b2Body, b2Force, true);
                break;
        }
    }

    void Animator2DComponent::SwitchToAnimation(s32 animSlot)
    {
        AnimationController* animController = AssetManager::GetAsset<AnimationController>(controller);

        if (animController != NULL && animSlot >= 0 && animSlot <= animController->animations.size() - 1)
            activeSlot = animSlot;
    }
}
