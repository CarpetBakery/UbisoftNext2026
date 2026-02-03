#ifndef _PARTICLES_2D
#define _PARTICLES_2D

#include <engine/ecs/System.h>
#include <engine/Graphics.h>
#include <engine/Math.h>
#include <engine/Spatial.h>

#include <vector>

#include <engine/components/Transform2D.h>

namespace GS
{
    // A particle system
    struct Particles2D
    {
        struct Particle
        {
            float lifeTime = 1.0f;
            float time = 0.0f;

            Engine::Vec2f pos = Engine::Vec2f(0, 0);
            Engine::Vec2f velocity = Engine::Vec2f(0, 0);

            float rotation = 0.0f;
            float rotationSpd = 0.0f;

            Engine::Sprite *spr = nullptr;

            // Animatable
            Engine::Vec2f scale = Engine::Vec2f(1.0f, 1.0f);
            Engine::Vec2f scaleStart = Engine::Vec2f(1.0f, 1.0f);
            Engine::Vec2f scaleEnd = Engine::Vec2f(1.0f, 1.0f);

            Engine::Color color = Engine::Color::white;
            Engine::Color colorStart = Engine::Color::white;
            Engine::Color colorEnd = Engine::Color::white;
        };

        std::vector<Particle> particles;
        
        // List of random sprites to pick from
        std::vector<Engine::Sprite *> spritePool;

        // Is it an explosion of particles, or a steady stream?
        bool oneShot = false;

        // Should we scale particles non-uniformly
        bool nonUniformScaling = true;

        // If oneShot, number of particles to explode on init
        // if not, it's particles per second
        int particleCount = 10;

        // System lifetime in seconds (doesn't matter if we're oneshot)
        float lifeTime = 0.0f;
        float time = 0.0f;

        // -- Particle parameters --
        // Particle gravity
        float gravity = 2.0f;
        Engine::Vec2f gravityDir = Engine::Vec2f(0.0f, 1.0f);

        // Particle lifetime (seconds)
        float particleLifeTimeMin = 1.0f;
        float particleLifeTimeMax = 1.5f;

        float rotationMin = 0.0f;
        float rotationMax = 2.0f * Engine::Math::pi;

        float rotationSpeedMin = 0.0f;
        float rotationSpeedMax = 0.0f;

        Engine::Vec2f velocityMin = Engine::Vec2f(0, 0);
        Engine::Vec2f velocityMax = Engine::Vec2f(0, 0);

        // Animatable properties
        float scaleStartMin = 1.0f;
        float scaleStartMax = 1.0f;

        float scaleEndMin = 1.0f;
        float scaleEndMax = 1.0f;


        Engine::Color colorStart = Engine::Color::white;
        Engine::Color colorEnd = Engine::Color::white;
    };

    class UpdateParticles2D : public Engine::System
    {
    public:
        void init() override;
        void entityAdded(Engine::Entity const &ent) override;
        void update() override;

        void spawnParticle(Particles2D &pSystem, Transform2D &transform);
    };

    class DrawParticles2D : public Engine::System
    {
    public:
        void init() override;
        void update() override;
    };

    namespace Particles2DCommon
    {
        // Signal system to stop emitting and destroy
        void destroy(Particles2D &particles);
    }
}


#endif // _PARTICLES_2D