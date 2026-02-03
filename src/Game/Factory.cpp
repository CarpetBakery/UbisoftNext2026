#include "Factory.h"

#include <engine/Audio.h>
#include <engine/Spatial.h>
#include <engine/Time.h>

#include <GameplayComponents.h>

#include <Content.h>

using namespace Engine;
using namespace GS;

Entity Factory::solid(Scene *scene, const Vec2i &pos, const Vec2i &shape)
{
	Entity ent = scene->createEntity();
	Solid solid;
	Transform2D transform;
	Collider2D collider;

	transform.pos = pos;
	collider.rect = Recti(pos, shape);

	scene->addComponent(ent, solid);
	scene->addComponent(ent, collider);
	scene->addComponent(ent, transform);

	return ent;
}

Entity Factory::solidGrapplable(Scene *scene, const Vec2i &pos, const Vec2i &shape)
{
	Entity ent = solid(scene, pos, shape);

	Grapplable grapplable;
	scene->addComponent(ent, grapplable);

	return ent;
}

Entity Factory::player(Scene *scene, const Vec2i &pos)
{
	constexpr Vec2i spriteOrigin = Vec2i(11, 28);
	Entity ent = scene->createEntity();

	Mover2D mover;
	UID uid;
	Player player;
	Solid solid;
	Animator animator;
	Transform2D transform;
	Collider2D collider;
	Shadow shadow;
	Gravity gravity;

	animator.sprite = &Content::sprPlayer;
	animator.lerpOffset = true;
	animator.lerpOffsetFac = 0.4;
	AnimatorCommon::setOrigin(animator, spriteOrigin);

	transform.pos = pos;

	float colliderHeight = animator.sprite->GetHeight() * 0.6f;
	collider.rect = Recti(
		pos.x - spriteOrigin.x,
		pos.y - spriteOrigin.y + (animator.sprite->GetHeight() - colliderHeight),
		animator.sprite->GetWidth(), colliderHeight);

	shadow.offset = Vec2i(11, 33);

	scene->addComponent(ent, transform);
	scene->addComponent(ent, collider);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, mover);
	scene->addComponent(ent, uid);
	scene->addComponent(ent, player);
	scene->addComponent(ent, solid);
	scene->addComponent(ent, shadow);
	scene->addComponent(ent, gravity);

	return ent;
}

Engine::Entity GS::Factory::cameraController(Engine::Scene *scene, const Engine::Vec2i &pos, const Vec2i &screenBounds)
{
	Entity ent = scene->createEntity();

	Transform2D transform;
	transform.pos = pos;

	CameraController camController;
	camController.screenBounds = screenBounds;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, camController);

	return ent;
}

Engine::Entity GS::Factory::coin(Engine::Scene *scene, const Engine::Vec2i &pos)
{
	Entity ent = scene->createEntity();

	Shadow shadow;

	Coin coin;
	coin.animOffset = Time::seconds;

	Transform2D transform;
	transform.pos = pos;

	Animator animator;
	animator.sprite = &Content::sprCoin;
	AnimatorCommon::setOrigin(animator, Sprite::Origin::BottomMiddle);

	Collider2D collider;
	collider.rect = Recti(
		pos - animator.sprite->GetSize() / 2,
		animator.sprite->GetSize());

	Gravity gravity;
	gravity.zVelocity = -15.0f;
	gravity.peakVelocity = gravity.zVelocity;

	Mover2D mover;
	float spd = 5.0f;
	mover.velocity = Vec2f(
		Math::randRange(-spd, spd),
		Math::randRange(-spd, spd));

	// Bounce on collision
	auto xCallback = [ent, scene](Entity collidingEntity)
		{
			auto &coin = scene->getComponent<Coin>(ent);
			
			// Pickup coins
			if (scene->hasComponent<Player>(collidingEntity) && coin.destroytimer > 200)
			{
				scene->queueDestroy(ent);
			}

			auto &mover = scene->getComponent<Mover2D>(ent);
			mover.velocity.x = mover.velocityBeforeCollision.x * -1;
		};
	auto yCallback = [ent, scene](Entity collidingEntity)
		{
			auto &coin = scene->getComponent<Coin>(ent);

			// Pickup coins
			if (scene->hasComponent<Player>(collidingEntity) && coin.destroytimer > 200)
			{
				scene->queueDestroy(ent);
			}

			auto &mover = scene->getComponent<Mover2D>(ent);
			mover.velocity.y = mover.velocityBeforeCollision.y * -1;
		};
	Mover2DCommon::setOnCollideX(mover, xCallback);
	Mover2DCommon::setOnCollideY(mover, yCallback);

	scene->addComponent(ent, shadow);
	scene->addComponent(ent, coin);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, transform);
	scene->addComponent(ent, collider);
	scene->addComponent(ent, mover);
	scene->addComponent(ent, gravity);

	return ent;
}

Engine::Entity GS::Factory::ceilingHook(Engine::Scene *scene, const Engine::Vec2i &pos, int z, bool intro)
{
	Entity ent = scene->createEntity();

	CeilingHook ceilHook;
	Transform2D transform;
	Collider2D collider;
	Animator animator;
	Grapplable grapplable;
	Shadow shadow;
	UID uid;

	ceilHook.intro = intro;

	animator.sprite = &Content::sprCeilingHook;
	AnimatorCommon::setOrigin(animator, Sprite::Origin::BottomMiddle);

	animator.lerpOffset = true;
	animator.lerpOffsetFac = 0.4;

	transform.pos = pos;
	transform.z = z;

	collider.rect = Recti(
		pos - Vec2i(animator.sprite->GetWidth() / 2, animator.sprite->GetHeight() + transform.z),
		animator.sprite->GetSize());

	scene->addComponent(ent, ceilHook);
	scene->addComponent(ent, transform);
	scene->addComponent(ent, collider);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, grapplable);
	scene->addComponent(ent, shadow);
	scene->addComponent(ent, uid);

	return ent;
}

Engine::Entity GS::Factory::transition(Engine::Scene *scene, int type, std::function<void(void)> callback)
{
	Entity ent = scene->createEntity();

	Transition transition;
	transition.type = type;
	transition.callback = callback;

	scene->addComponent(ent, transition);

	return ent;
}

Engine::Entity GS::Factory::enemy(Engine::Scene *scene, const Engine::Vec2i &pos)
{
	Entity ent = scene->createEntity();

	Enemy enemy;
	Animator animator;
	Collider2D collider;
	Transform2D transform;
	Grapplable grapplable;
	Shadow shadow;

	enemy.spawnTime = Time::seconds;
	enemy.moveSpd = Math::randRange(2.0f, 5.5f);

	transform.pos = pos;

	animator.sprite = &Content::sprSkull;
	AnimatorCommon::setOrigin(animator, Sprite::Origin::BottomMiddle);

	collider.rect = Recti(pos - Vec2i(animator.sprite->GetWidth() / 2, animator.sprite->GetHeight()),
		animator.sprite->GetSize());

	scene->addComponent(ent, enemy);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, collider);
	scene->addComponent(ent, transform);
	scene->addComponent(ent, grapplable);
	scene->addComponent(ent, shadow);

	Factory::smokeExplosion(scene, pos, 5);

	return ent;
}

Engine::Entity GS::Factory::enemyGen(Engine::Scene *scene)
{
	Entity ent = scene->createEntity();

	scene->addComponent(ent, EnemyGen());
	
	return ent;
}

Entity Factory::smokeExplosion(Scene *scene, const Vec2i &pos, int particleCount)
{
	Entity ent = scene->createEntity();

	Transform2D transform;
	transform.pos = pos;

	Particles2D pSystem;
	pSystem.spritePool = {
		&Content::sprSmoke1,
		&Content::sprSmoke2 };

	pSystem.particleCount = particleCount;

	pSystem.particleLifeTimeMin = 20.0f / 60.0f;
	pSystem.particleLifeTimeMax = 50.0f / 60.0f;

	pSystem.velocityMin = Vec2f(-3, 0);
	pSystem.velocityMax = Vec2f(3, 3);

	pSystem.gravity = 0.1f;
	pSystem.gravityDir = Vec2f(0.0, -1.0f);

	pSystem.rotationSpeedMin = -10.0f;
	pSystem.rotationSpeedMax = 10.0f;

	pSystem.scaleStartMin = 0.9f;
	pSystem.scaleEndMin = 0.9f;

	pSystem.scaleStartMax = 1.4f;
	pSystem.scaleEndMax = 1.4f;

	pSystem.colorEnd = Color::white;
	pSystem.colorEnd.a = 0;

	pSystem.oneShot = true;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, pSystem);

	return ent;
}

Entity Factory::hookEffect(Scene *scene, const Vec2i &pos)
{
	Entity ent = scene->createEntity();

	Transform2D transform;
	transform.pos = pos;

	Particles2D pSystem;
	pSystem.spritePool = {
		&Content::sprCeilingHookEffect };

	pSystem.particleCount = 1;

	pSystem.particleLifeTimeMin = 0.25f;
	pSystem.particleLifeTimeMax = pSystem.particleLifeTimeMin;

	pSystem.gravity = 0.0f;

	pSystem.rotationSpeedMin = 15.0f;
	pSystem.rotationSpeedMax = pSystem.rotationSpeedMin;

	pSystem.scaleStartMin = 1.0f;
	pSystem.scaleStartMax = pSystem.scaleStartMin;

	pSystem.scaleEndMin = 6.0f;
	pSystem.scaleEndMax = pSystem.scaleEndMin;

	pSystem.colorEnd = Color::white;
	pSystem.colorEnd.a = 0;

	pSystem.oneShot = true;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, pSystem);

	return ent;
}

Engine::Entity GS::Factory::shockwaveEffect(Engine::Scene *scene, const Engine::Vec2i &pos)
{
	Entity ent = scene->createEntity();

	Transform2D transform;
	transform.pos = pos;

	Particles2D pSystem;
	pSystem.spritePool = {
		&Content::sprCursorOuter };

	pSystem.particleCount = 1;

	pSystem.particleLifeTimeMin = 0.2f;
	pSystem.particleLifeTimeMax = pSystem.particleLifeTimeMin;

	pSystem.gravity = 0.0f;

	pSystem.rotationSpeedMin = 15.0f;
	pSystem.rotationSpeedMax = pSystem.rotationSpeedMin;

	pSystem.scaleStartMin = 1.0f;
	pSystem.scaleStartMax = pSystem.scaleStartMin;

	pSystem.scaleEndMin = 8.0f;
	pSystem.scaleEndMax = pSystem.scaleEndMin;

	pSystem.colorEnd = Color::white;
	pSystem.colorEnd.a = 0;

	pSystem.oneShot = true;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, pSystem);

	return ent;

	return Engine::Entity();
}
