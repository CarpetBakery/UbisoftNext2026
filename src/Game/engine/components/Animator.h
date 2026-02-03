#ifndef _ANIMATOR_H
#define _ANIMATOR_H

#include <engine/Spatial.h>
#include <engine/Sprite.h>
#include <engine/Graphics.h>
#include <engine/ecs/System.h>

struct Animator
{
    Engine::Sprite *sprite = nullptr;
    int animation = 0;
    
    // Offset to draw at
    Engine::Vec2f offset = Engine::Vec2f(0, 0);
    bool lerpOffset = false;
    float lerpOffsetFac = 0.4;
    
    // Sprite origin
    Engine::Sprite::Origin origin = Engine::Sprite::Origin::TopLeft;
    Engine::Vec2i originCustom = Engine::Vec2i(0, 0);

    Engine::Color color = Engine::Color::fromFloat(1.0f, 1.0f, 1.0f, 1.0f);

    // For depth sorting
    int depth = 0;
};

class UpdateAndDrawAnimator : public Engine::System
{
public:
    void init() override;
    void update() override;
};

namespace AnimatorCommon
{
    void setOrigin(Animator &animator, Engine::Sprite::Origin origin);
    void setOrigin(Animator &animator, Engine::Vec2i origin);
}

#endif // _ANIMATOR_H