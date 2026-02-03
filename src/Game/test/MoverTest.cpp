#include "MoverTest.h"

#include <engine/Ecs.h>

#include <Content.h>
#include <Factory.h>

#include <engine/components/Transform2D.h>
#include <engine/components/Collider2D.h>
#include <engine/components/Animator.h>
#include <components/Mover2D.h>
#include <components/Particles2D.h>
#include <components/Solid.h>
#include <components/UID.h>

#include <test/components/TestBox.h>

using namespace Engine;
using namespace GS;

namespace
{
    constexpr int wallThickness = 16;
    constexpr float minSpd = -4.0f;
    constexpr float maxSpd = 4.0f;
    constexpr float spdMult = 1.0f;
    
    void initMovers(Scene *scene)
    {
        // Components
        scene->registerComponent<Mover2D>();
        scene->registerComponent<Transform2D>();
        scene->registerComponent<Collider2D>();
        scene->registerComponent<Particles2D>();
        scene->registerComponent<TestBox>();

        // Systems
        // NOTE: Can't have both of these running at the same time...
        // because mover entities will be captured by both
        scene->registerSystem<UpdateMoveAndCollide2D>();
        // scene->registerSystem<UpdateMover2D>();

        scene->registerSystem<UpdateTestBox>();
        scene->registerSystem<DrawCollider2D>();

        // Create entites
        Factory::createTestBox(scene, Recti(200, 200, 64, 64));
        Factory::createTestBox(scene, Recti(200, 400, 64, 64));
        Factory::createTestBox(scene, Recti(10, 400, 64, 64));

        {
            Entity ent = scene->createEntity();

            Collider2D collider;
            collider.rect = Recti(300, 300, 40, 40);

            scene->addComponent(ent, collider);
        }
    }

    Entity createBouncer(Scene *scene, const Vec2i &pos, const Vec2i &colSize, const Vec2i &velocity)
    {
        Entity ent = scene->createEntity();

        Transform2D transform;
        transform.pos = pos;

        Collider2D collider;
        collider.rect = Recti(transform.pos, colSize);

        Mover2D mover;
        mover.velocity = velocity;

        Animator animator;
        animator.sprite = &Content::sprEventMark;

        UID uid;

        // Bounce on collision
        auto xCallback = [scene](Entity ent)
            {
                auto &mover = scene->getComponent<Mover2D>(ent);
                mover.velocity.x = mover.velocityBeforeCollision.x * -1;
            };
        auto yCallback = [scene](Entity ent)
            {
                auto &mover = scene->getComponent<Mover2D>(ent);
                mover.velocity.y = mover.velocityBeforeCollision.y * -1;
            };

        Mover2DCommon::setOnCollideX(mover, xCallback);
        Mover2DCommon::setOnCollideY(mover, yCallback);

        scene->addComponent(ent, mover);
        scene->addComponent(ent, transform);
        scene->addComponent(ent, collider);
        scene->addComponent(ent, animator);
        scene->addComponent(ent, uid);

        return ent;
    }

    void initBouncers(Scene *scene)
    {
        // Components
        scene->registerComponent<Animator>();
        scene->registerComponent<Mover2D>();
        scene->registerComponent<Transform2D>();
        scene->registerComponent<Collider2D>();
        scene->registerComponent<Solid>();
        scene->registerComponent<UID>();

        // Systems
        scene->registerSystem<UpdateAndDrawAnimator>();
        scene->registerSystem<UpdateMoveAndCollide2D>();
        scene->registerSystem<UpdateSolids>();
        scene->registerSystem<UpdateUID>();
        // scene->registerSystem<DrawCollider2D>();

        // Entities

        // Create walls
        constexpr int wallThickness = 16;
        const int screenWidth = scene->m_game->getScreenWidth();
        const int screenHeight = scene->m_game->getScreenHeight();

        // Left wall
        Factory::solid(scene,
                             Vec2i(0, 0),
                             Vec2i(16, screenHeight));
        // Right wall
        Factory::solid(scene,
                             Vec2i(screenWidth - wallThickness, 0),
                             Vec2i(wallThickness, screenHeight));
        // Top wall
        Factory::solid(scene,
                             Vec2i(0, 0),
                             Vec2i(screenWidth, wallThickness));
        // Bottom wall
        Factory::solid(scene,
                             Vec2i(0, screenHeight - wallThickness),
                             Vec2i(screenWidth, wallThickness));

        // Create bouncers
        constexpr int numBouncers = 2;
        for (int i = 0; i < numBouncers; i++)
        {
            Vec2i colSize = Vec2i(Math::randRangei(8, 32), Math::randRangei(8, 32));

            createBouncer(scene,
                          Vec2i(Math::randRange(wallThickness, screenWidth - wallThickness - colSize.x), Math::randRange(wallThickness, screenHeight - wallThickness - colSize.y)),
                          colSize,
                          Vec2i(Math::randRange(minSpd, maxSpd) * spdMult, Math::randRange(minSpd, maxSpd) * spdMult));
        }
    }
}

void MoverTestScene::init()
{
    // initMovers(this);
    initBouncers(this);
}

void MoverTestScene::update(const float dt)
{
    Scene::update(dt);

    // Testing out UIDs
    if (m_game->m_input.keyPressed(App::KEY_E))
    {
        auto *uidManager = getSystem<UpdateUID>();
        if (uidManager->m_entities.size() > 0)
        {
            if (uidManager->entityExists(9999))
            {
                auto ent = uidManager->getEntity(9999);
                destroyEntity(ent);
                printf("Removed UID entity\n");
            }
        }
    }

    if (m_game->m_input.keyPressed(App::KEY_Q))
    {
        const int screenWidth = m_game->getScreenWidth();
        const int screenHeight = m_game->getScreenHeight();
        
        Vec2i colSize = Vec2i(Math::randRangei(8, 32), Math::randRangei(8, 32));
        
        createBouncer(this,
                      Vec2i(Math::randRange(wallThickness, screenWidth - wallThickness - colSize.x), Math::randRange(wallThickness, screenHeight - wallThickness - colSize.y)),
                      colSize,
                      Vec2i(Math::randRange(minSpd, maxSpd) * spdMult, Math::randRange(minSpd, maxSpd) * spdMult));
    }

    // Play a song with space
    if (m_game->m_input.keyPressed(App::KEY_SPACE))
    {
        if (m_game->m_tritonePlayer.isPlaying())
        {
            m_game->m_tritonePlayer.pause();
        }
        else
        {
            m_game->m_tritonePlayer.loadAndPlay("enegy.tri");
        }
    }
}
