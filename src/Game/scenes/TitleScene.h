#ifndef _TITLE_SCENE_H
#define _TITLE_SCENE_H

#include <engine/ecs/Scene.h>

class TitleScene : public Engine::Scene
{
public:
    void init() override;
    void draw() override;
};


#endif // _TITLE_SCENE_H