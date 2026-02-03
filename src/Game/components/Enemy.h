#ifndef _ENEMY_H
#define _ENEMY_H

#include <engine/ecs/System.h>
#include <engine/ecs/Scene.h>

namespace GS
{
    struct Enemy
    {
        float spawnTime = 0.0f;
        float moveSpd = 4.0f;
    };

    class UpdateEnemy : public Engine::System
    {
    public:
        void init() override;
        void update() override;
    };

    namespace EnemyCommon
    {
        void kill(Engine::Scene *scene, Engine::Entity enemyEnt);
    }
}

#endif // _ENEMY_H