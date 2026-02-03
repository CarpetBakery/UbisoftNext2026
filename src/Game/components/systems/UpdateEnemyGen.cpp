#include <components/EnemyGen.h>

#include <engine/Audio.h>
#include <engine/Time.h>
#include <engine/Math.h>

#include <GameplayComponents.h>

#include <Content.h>
#include <Factory.h>

using namespace Engine;
using namespace GS;

namespace
{
    float spawnRadius = 70.0f;
}

void GS::UpdateEnemyGen::init()
{
    Signature sig;

    sig.set(m_scene->getComponentType<EnemyGen>());
    
    setup(0, Type::Update, sig);
}

void GS::UpdateEnemyGen::update()
{
    auto *playerSys = m_scene->getSystem<UpdatePlayer>();
    Recti col = Recti(0, 0, Content::sprSkull.GetWidth(), Content::sprSkull.GetWidth());

    if (playerSys->m_entities.size() <= 0)
    {
        return;
    }

    Entity playerEnt = *playerSys->m_entities.begin();
    auto &pTransform = m_scene->getComponent<Transform2D>(playerEnt);
    auto &pCol = m_scene->getComponent<Collider2D>(playerEnt);
    
    for (auto const &ent : m_entities)
    {
        auto &enemyGen = m_scene->getComponent<EnemyGen>(ent);

        if (Time::onInterval(enemyGen.interval))
        {
            Vec2f spawnPos;
            do
            {
                spawnPos = Vec2f(
                    Math::randRange(0, m_game->getScreenWidth()),
                    Math::randRange(0, m_game->getScreenHeight()));
    
                // col.x = spawnPos.x;
                // col.y = spawnPos.y;

            } while ((spawnPos - pTransform.pos).length() < spawnRadius);

            Factory::enemy(m_scene, spawnPos);
        }

        enemyGen.interval -= Time::deltaSeconds * 0.01;
        printf("%f\n", enemyGen.interval);
    }
}