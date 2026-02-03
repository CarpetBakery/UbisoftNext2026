#include "GameScene.h"

#include <engine/Ecs.h>

#include <engine/Audio.h>
#include <engine/Time.h>

#include <Content.h>
#include <Factory.h>
#include <GameplayComponents.h>


using namespace Engine;
using namespace GS;

void GameScene::init()
{
	registerGameplayComponents(this);

    int sw = m_game->getScreenWidth();
    int sh = m_game->getScreenHeight();

    int rw = Content::sprMainBg.GetWidth();
    int rh = Content::sprMainBg.GetHeight();

    Factory::transition(this, 0);

    Factory::player(this, Vec2i(sw, sh) / 2);
    Factory::cameraController(this, Vec2i(sw, sh) / 2, Vec2i(sw * 1.5f, sh * 1.5f));

    Factory::ceilingHook(this, Vec2i(rw/2 - rw/4, rh/2 - rh/4), 150);
    Factory::ceilingHook(this, Vec2i(rw/2 + rw/4, rh/2 - rh/4), 150);

    Factory::enemyGen(this);

    // -- Walls --

    {
        int wallWidth = 88;
        int wallHeight = 72;
        
        // Left
        Factory::solidGrapplable(this, Vec2i(0, 0), Vec2i(wallWidth, rh));
        // Right
        Factory::solidGrapplable(this, Vec2i(rw - wallWidth, 0), Vec2i(wallWidth, rh));
        // Top 
        Factory::solidGrapplable(this, Vec2i(0, 0), Vec2i(rw, wallHeight));
        // Bot
        Factory::solidGrapplable(this, Vec2i(0, rh - wallHeight), Vec2i(rw, wallHeight));

        // Middle walls
        Factory::solidGrapplable(this, Vec2i(700, 520), Vec2i(520, 40));
        // Factory::solidGrapplable(this, Vec2i(942, 304), Vec2i(36, 472));
        Factory::solidGrapplable(this, Vec2i(890, 472), Vec2i(140, 136));
    }
}

void GameScene::update(const float dt)
{
    if (m_game->m_input.mRightPressed())
    {
        Factory::enemy(this, m_game->m_input.mousePos());
        //Factory::smokeExplosion(this, m_game->m_input.mousePos(), 5);
    }

    Scene::update(dt);
}

void GameScene::draw()
{
    Content::sprMainBgBehind.DrawEx(-Vec2f(m_camera.m_pos) * 0.2f);
    Content::sprMainBg.DrawExCam(Vec2i(0, 0));

    Scene::draw();
}
