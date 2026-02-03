#ifndef _FACTORY_H
#define _FACTORY_H

#include <engine/ecs/Scene.h>
#include <engine/Spatial.h>
#include <functional>

namespace GS
{
    namespace Factory
    {
        Engine::Entity solid(Engine::Scene *scene, const Engine::Vec2i &pos, const Engine::Vec2i &shape);
        Engine::Entity solidGrapplable(Engine::Scene *scene, const Engine::Vec2i &pos, const Engine::Vec2i &shape);

        // Objects
        Engine::Entity player(Engine::Scene *scene, const Engine::Vec2i &pos);
        Engine::Entity cameraController(Engine::Scene *scene, const Engine::Vec2i &pos, const Engine::Vec2i &screenBounds);
        Engine::Entity coin(Engine::Scene *scene, const Engine::Vec2i &pos);
        Engine::Entity ceilingHook(Engine::Scene *scene, const Engine::Vec2i &pos, int z, bool intro = false);
        Engine::Entity transition(Engine::Scene *scene, int type, std::function<void(void)> callback = nullptr);
        Engine::Entity enemy(Engine::Scene *scene, const Engine::Vec2i &pos);
        Engine::Entity enemyGen(Engine::Scene *scene);

        // Effects
        Engine::Entity smokeExplosion(Engine::Scene *scene, const Engine::Vec2i &pos, int particleCount);
        Engine::Entity hookEffect(Engine::Scene *scene, const Engine::Vec2i &pos);
        Engine::Entity shockwaveEffect(Engine::Scene *scene, const Engine::Vec2i &pos);
    }
}

#endif // _FACTORY_H