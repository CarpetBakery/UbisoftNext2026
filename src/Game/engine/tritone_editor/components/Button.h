#ifndef _BUTTON_H
#define _BUTTON_H

#include <engine/ecs/System.h>

#include <functional>

struct Button
{
    // Button won't respond to input if disabled
    bool disabled = false;

    // Button auto-switches to pressed animation when pressed
    bool showPressedAnim = true;
    
    using Callback = std::function<void(Engine::Entity const &)>;
    Callback pressedCallback;
};

class UpdateButton : public Engine::System
{
public:
    void init() override;
    void update() override;
};

void setButtonBounds(Engine::Scene *scene, Engine::Entity ent, const Engine::Vec2i &pos, const Engine::Vec2i &size = Engine::Vec2i(-1, -1));

#endif // _BUTTON_H