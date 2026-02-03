#ifndef _PLAYER_TEST_SCENE_H
#define _PLAYER_TEST_SCENE_H

#include <engine/ecs/Scene.h>

class PlayerTestScene : public Engine::Scene
{
public:
    void init() override;
    void update(const float dt) override;
};

#endif // _PLAYER_TEST_SCENE_H