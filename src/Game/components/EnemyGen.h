#ifndef _ENEMY_GEN_H
#define _ENEMY_GEN_H

#include <engine/ecs/System.h>

namespace GS
{
    struct EnemyGen
    {
        float interval = 0.9f;
    
    };

    class UpdateEnemyGen : public Engine::System
    {
    public:
        void init() override;
        void update() override;
    };
}


#endif // _ENEMY_GEN_H