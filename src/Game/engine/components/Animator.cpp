#include "Animator.h"

using namespace Engine;

void AnimatorCommon::setOrigin(Animator &animator, Engine::Sprite::Origin origin)
{
    animator.originCustom = Vec2i(0, 0);
    animator.origin = origin;
}

void AnimatorCommon::setOrigin(Animator &animator, Engine::Vec2i origin)
{
    animator.originCustom = origin;
    animator.origin = Sprite::Origin::Custom;
}
