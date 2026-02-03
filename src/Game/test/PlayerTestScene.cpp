#include "PlayerTestScene.h"

#include <engine/Time.h>
#include <engine/Audio.h>

#include <GameplayComponents.h>
#include <Factory.h>
#include <Content.h>

using namespace Engine;
using namespace GS;

void PlayerTestScene::init()
{
    registerGameplayComponents(this);

    // -- Entities --
    Factory::player(this, m_game->getScreenSize() / 2);
    // Factory::player(this, m_game->getScreenSize() / 2 + Vec2i(80, 0));
    // Factory::player(this, m_game->getScreenSize() / 2 + Vec2i(120, 50));

    Factory::cameraController(this, m_game->getScreenSize() / 2, m_game->getScreenSize() * 2);

    // Create solid walls
    constexpr int wallWidth = 20;
    Factory::solidGrapplable(this, Vec2i(0, 0), Vec2i(wallWidth, m_game->getScreenHeight()));
    Factory::solidGrapplable(this, Vec2i(0, 0), Vec2i(m_game->getScreenWidth(), wallWidth));
    Factory::solidGrapplable(this, Vec2i(200, 200), Vec2i(48, 48));

    // Create ceiling hook
    Factory::ceilingHook(this, Vec2i(400, 600), 200);

    Factory::transition(this, 0);

    // Depth tester guy
    {
        Entity ent = createEntity();

        Transform2D transform;
        transform.pos = Vec2i(400, 200);

        constexpr Vec2i spriteOrigin = Vec2i(11, 28);
        Animator animator;
        animator.sprite = &Content::sprPlayer;
        AnimatorCommon::setOrigin(animator, spriteOrigin);
        animator.depth = transform.pos.y;

        Shadow shadow;
        shadow.offset = Vec2i(11, 33);

        Collider2D collider;
        collider.rect = Recti(transform.pos, Vec2i(24, 24));

        Solid solid;

        addComponent(ent, animator);
        addComponent(ent, transform);
        addComponent(ent, shadow);
        addComponent(ent, collider);
        addComponent(ent, solid);
    }

    // Print scene info
    printf("%s\n", toString().c_str());

    // TODO: Remove this
    glClearColor(0.6, 0.6, 0.8, 1.0);
}

// DEBUG
void PlayerTestScene::update(const float dt)
{
    // Create explosions
    if (m_game->m_input.mRight())
    {
        // Factory::smokeExplosion(this, m_game->m_input.mousePos(), 10);
        Factory::coin(this, m_game->m_input.mousePos());

        // Entity ent = Factory::coin(this, m_game->m_input.mousePos());
        // auto &coin = getComponent<Coin>(ent);
        // coin.destroytimer = 30.0f;
    }
    
    // if (m_game->m_input.mRightPressed())
    // {
    //     Audio::play(Content::sndHookEffect);
    //     Factory::hookEffect(this, m_game->m_input.mousePos());
    // }

    if (m_game->m_input.keyPressed(App::KEY_1))
    {
        auto &entities = getSystem<UpdatePlayer>()->m_entities;
        if (entities.size() > 0)
        {
            queueDestroy(*entities.begin());
        }
    }

    // Move camera with arrow keys
    Vec2f inputAxis = m_game->m_input.getAxis(
        App::KEY_LEFT,
        App::KEY_RIGHT,
        App::KEY_UP,
        App::KEY_DOWN);
    m_camera.m_pos += inputAxis * 2.0f * Time::delta;

    Scene::update(dt);
}
