#ifndef _EXAMPLE_SCENE_H
#define _EXAMPLE_SCENE_H

#include <engine/ecs/Scene.h>

namespace GS
{
    class ExampleScene : public Engine::Scene
    {
        void init() override;
        void update(const float dt) override;
        void draw() override;
        void destroyed() override;
    };
}

#endif // _EXAMPLE_SCENE_H