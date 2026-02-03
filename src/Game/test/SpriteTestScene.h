#ifndef _SPRITE_TEST_SCENE
#define _SPRITE_TEST_SCENE

#include <engine/ecs/Scene.h>

namespace GS
{
    class SpriteTestScene : public Engine::Scene
    {
    public:
        void init() override;
    };
}

#endif // _SPRITE_TEST_SCENE