#include <components/CeilingHook.h>

#include <engine/Ecs.h>

#include <components/Player.h>
#include <engine/components/Transform2D.h>
#include <Factory.h>
#include <test/PlayerTestScene.h>
#include <scenes/GameScene.h>

using namespace Engine;
using namespace GS;

void UpdateCeilingHook::init()
{
    Signature sig;

    sig.set(m_scene->getComponentType<CeilingHook>());
    sig.set(m_scene->getComponentType<Transform2D>());

    setup(0, Type::Update, sig);
}

void UpdateCeilingHook::update()
{
    auto *playerSys = m_scene->getSystem<UpdatePlayer>();

    for (auto const &ent : m_entities)
    {
        auto &ceilHook = m_scene->getComponent<CeilingHook>(ent);
        auto &transform = m_scene->getComponent<Transform2D>(ent);

        if (!ceilHook.intro)
        {
            continue;
        }

        if (playerSys->m_entities.size() <= 0)
        {
            continue;
        }

        Entity playerEnt = *playerSys->m_entities.begin();
        auto &player = m_scene->getComponent<Player>(playerEnt);
        
        if (player.state == Player::State::CeilingHook && !ceilHook.startIntro)
        {
            ceilHook.startIntro = true;

            Game *game = m_game;
            Factory::transition(m_scene, 1, [game]() {
                // game->m_tritonePlayer.loadAndPlay("voltest_low.tri");
                //game->pushScene<GameScene>();

                game->setScene<GameScene>();
                game->m_tritonePlayer.loadAndPlay("enegy_otherbass.tri");
             });
        }
    }
}