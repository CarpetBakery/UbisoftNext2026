#ifndef _MOVER_TEST_H
#define _MOVER_TEST_H

#include <engine/ecs/Scene.h>

namespace GS
{
    // Simple scene with a lot of movers
    class MoverTestScene : public Engine::Scene
    {
    public:
        void init() override;
        void update(const float dt) override;
    };
}

#endif // _MOVER_TEST_H