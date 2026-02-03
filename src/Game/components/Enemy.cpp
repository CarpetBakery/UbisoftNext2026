#include "Enemy.h"

#include <engine/Audio.h>

#include <GameplayComponents.h>
#include <Content.h>
#include <Factory.h>

using namespace Engine;
using namespace GS;

void GS::EnemyCommon::kill(Engine::Scene *scene, Engine::Entity enemyEnt)
{
    auto &transform = scene->getComponent<Transform2D>(enemyEnt);
    int amt = Math::randRangei(2, 4);
    if (amt < 2)
    {
        printf("What the hell\n");
    }
    for (int i = 0; i < amt; i++)
    {
        Factory::coin(scene, transform.pos - Vec2i(0, 20));
    }

    scene->m_camera.shake(20);
    Factory::shockwaveEffect(scene, transform.pos);
    Audio::play(Content::sndDeath);

    // Do hitstun
    scene->setHitstun(0.06);

    scene->queueDestroy(enemyEnt);
}