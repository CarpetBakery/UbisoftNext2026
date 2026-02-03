#ifndef _GAME_SCENE_H
#define _GAME_SCENE_H

#include <engine/ecs/Scene.h>

class GameScene : public Engine::Scene
{
public:
    void init() override;
    void update(const float dt) override;
    void draw() override;
};

#endif // _GAME_SCENE_H