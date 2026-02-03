#include "TitleScene.h"

#include <engine/Ecs.h>
#include <engine/Time.h>
#include <engine/Audio.h>

#include <GameplayComponents.h>
#include <Factory.h>
#include <Content.h>

using namespace Engine;
using namespace GS;

void TitleScene::init()
{
    registerGameplayComponents(this);
    int sw = m_game->getScreenWidth();
    int sh = m_game->getScreenHeight();

    {
        Entity ent = Factory::player(this, (m_game->getScreenSize() / 2) + Vec2i(0, 200));
        auto &player = getComponent<Player>(ent);
        player.intro = true;
    }


    Factory::cameraController(this, Vec2i(0, 0), m_game->getScreenSize());
    
    // walls
    int wallWidth = 68;
    int wallHeight = 56;
    
    // Left
    Factory::solidGrapplable(this, Vec2i(0, 0), Vec2i(wallWidth, sh));
    // Right
    Factory::solidGrapplable(this, Vec2i(sw - wallWidth, 0), Vec2i(wallWidth, sh));
    // Top 
    Factory::solidGrapplable(this, Vec2i(0, 0), Vec2i(sw, wallHeight));
    // Bot
    Factory::solidGrapplable(this, Vec2i(0, sh - wallHeight), Vec2i(sw, wallHeight));

    // Start ceiling hook
    Factory::ceilingHook(this, Vec2i(sw/2, 350), 150, true);
}

void TitleScene::draw()
{
    Content::sprStartBg.DrawExCam(Vec2i(0, 0));
    
    Scene::draw();
}
