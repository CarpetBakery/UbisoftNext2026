#include <components/Player.h>

#include <engine/Audio.h>
#include <engine/Ecs.h>
#include <engine/Input.h>
#include <engine/Time.h>

#include <components/Grapplable.h>
#include <components/Shadow.h>
#include <components/CeilingHook.h>
#include <components/Enemy.h>

#include <Content.h>
#include <Factory.h>

using namespace Engine;
using namespace GS;
using namespace PlayerCommon;

namespace
{
	// Controls
	// TODO: Consider putting this in a configurable place
	constexpr App::Key bLeft = App::KEY_A;
	constexpr App::Key bRight = App::KEY_D;
	constexpr App::Key bUp = App::KEY_W;
	constexpr App::Key bDown = App::KEY_S;

	constexpr Input::Mouse bGrapple = Input::Mouse::Left;

	// Movement constants
	constexpr float maxSpd = 4.0f;
	constexpr float accel = 0.3f;
	constexpr float decel = 0.2f;

	constexpr float accelDamp = 0.10f;
	constexpr float decelDamp = 0.31f;
	constexpr float dampDtMult = 11.0f;

	// Grappling
	constexpr Vec2i grapplePlayerOffset = Vec2i(10, 0);
	constexpr float grappleSpd = 25.0f;
	constexpr float reelAccel = 1.2f;
	constexpr float bounceTime = 7.0f;
	constexpr float bounceSpd = 20.0f;

	constexpr float ceilHookBounceOffset = 80.0f;

	// Animation
	float walkBounceAmp = 7.0f * Graphics::resMult;
	constexpr float walkBounceFreq = 5.0f;

	float shakeAmount = 10.0f * Graphics::resMult;

	// Debug
	bool moveType = false;
}

void UpdatePlayer::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Collider2D>());
	sig.set(m_scene->getComponentType<Animator>());
	sig.set(m_scene->getComponentType<Mover2D>());
	sig.set(m_scene->getComponentType<Player>());
	sig.set(m_scene->getComponentType<UID>());

	setup(0, Type::Update, sig);
}

void UpdatePlayer::update()
{
	m_grapplableEntities = m_scene->getSystemEntities<UpdateGrapplable>();

	for (auto const &ent : m_entities)
	{
		PlayerEntity self = PlayerCommon::getComponents(m_scene, ent);

		switch (self.player->state)
		{
		case Player::State::Main:
			stMain(self);
			break;

		case Player::State::ShotGrapple:
			stShotGrapple(self);
			break;

		case Player::State::GrappleReeling:
			stGrappleReeling(self);
			break;

		case Player::State::GrappleBounce:
			stGrappleBounce(self);
			break;

		case Player::State::CeilingHook:
			stCeilingHook(self);
			break;
		
		case Player::State::Dead:
			if (m_game->m_input.keyPressed(App::KEY_R))
			{
				Scene *scene = m_scene;
				Game *game = m_game;
				Factory::transition(m_scene, 1, [scene, game](){
					game->restartScene();
				});
			}
			continue;
		}

		if (self.player->state != Player::State::Dead)
		{
			self.player->timeSurvived += Time::deltaSeconds;
		}

		checkEnemyCollision(self);

		// Set depth based on position
		self.animator->depth = self.transform->pos.y;

		// Scale cursor back down
		self.player->cursorScale = Math::damp(self.player->cursorScale, 1.0f, 0.2, Time::deltaSeconds * dampDtMult);
	}
}

void UpdatePlayer::stMain(PlayerEntity &self)
{
	handleMovement(self);
	handleGrappling(self);
}

void UpdatePlayer::stShotGrapple(PlayerEntity &self)
{
	handleMovement(self);

	// Grapple has been shot, but hasn't reached target yet
	self.player->grappleLine.a = self.transform->pos - Vec2i(0, self.transform->z) + grapplePlayerOffset;
	self.player->grappleLine.b = Math::approach(self.player->grappleLine.b, Vec2f(self.player->grappleTargetPoint), grappleSpd * Time::delta * Graphics::resMult);

	if (self.player->grappleLine.b == self.player->grappleTargetPoint)
	{
		// Grapple has landed
		startReeling(self);
	}
}

void UpdatePlayer::stGrappleReeling(PlayerEntity &self)
{
	// Reel 'em in
	self.player->reelSpeed += Time::delta * reelAccel;

	{
		Vec3f nrm = calcGrappleNormal(self);
		// Check signs
		// {
		// 	Vec2f v = self.player->grappleLine.b - self.player->grappleLine.a;
		// 	Vec2i newSigns = Vec2i(Math::sign(v.x), Math::sign(v.y));
		// 	if ((newSigns.x != 0 && newSigns.x != self.player->xSign) ||
		// 		(newSigns.y != 0 && newSigns.y != self.player->ySign))
		// 	{
		// 		doGrappleBounce(self);
		// 		return;
		// 	}
		// }
	}

	if (checkReelingCollision(self))
	{
		return;
	}

	// Give us velocity
	self.mover->velocity = self.player->grappleNormal * self.player->reelSpeed;
	self.gravity->zVelocity = self.player->elevationNormal * -self.player->reelSpeed * 2.0f;

	self.player->grappleLine.a = self.transform->pos - Vec2i(0, self.transform->z) + grapplePlayerOffset;

	if (self.player->grappleNormal == Vec2f(0, 0))
	{
		// We have zero velocity for some reason
		// Just instantly bounce instead of getting softlocked
		doGrappleBounce(self);
		return;
	}

	// if (Time::onInterval(0.009))
	// {
	// 	Factory::smokeExplosion(m_scene, self.transform->pos - Vec2i(0, self.transform->z) + grapplePlayerOffset, 3);
	// }

	if (Time::onInterval(0.02))
	{
		Factory::smokeExplosion(m_scene, self.transform->pos - Vec2i(0, self.transform->z) + grapplePlayerOffset, Math::randRangei(1, 2));
	}

	// Check for collision with objects

	// Make sure we don't overshoot our target
}

void UpdatePlayer::stGrappleBounce(PlayerEntity &self)
{
	handleMovement(self, true);

	self.player->stateTimer = Math::approach(self.player->stateTimer, 0.0f, Time::delta);
	if (self.player->stateTimer <= 0)
	{
		// Finished bouncing
		switchState(m_scene, self, Player::State::Main);
	}
}

void GS::UpdatePlayer::stCeilingHook(PlayerCommon::PlayerEntity &self)
{
	auto *uidSys = m_scene->getSystem<UpdateUID>();
	if (!uidSys->entityExists(self.player->ceilingHookUid))
	{
		switchState(m_scene, self, Player::State::Main);
		return;
	}

	// handleGrappling(self);
	if (m_game->m_input.mButtonPressed(bGrapple))
	{
		self.gravity->zVelocity = -10;
		switchState(m_scene, self, Player::State::Main);
		return;
	}

	Entity ceilHookEnt = uidSys->getEntity(self.player->ceilingHookUid);
	auto &hookTransform = m_scene->getComponent<Transform2D>(ceilHookEnt);

	{
		Vec2i colOffset = self.transform->pos - self.collider->rect.position();
		self.transform->pos = hookTransform.pos;
		self.collider->rect = Recti(
			self.transform->pos - colOffset,
			self.collider->rect.size());
	}
}

void UpdatePlayer::handleMovement(PlayerEntity &self, bool lockInput)
{
	Vec2f inputAxis = Vec2f(0, 0);
	if (!lockInput)
	{
		inputAxis = m_game->m_input.getAxis(bLeft, bRight, bUp, bDown);
	}

	Vec2f &velocity = self.mover->velocity;
	Vec2f &targetVelocity = self.player->targetVelocity;

	targetVelocity = inputAxis * maxSpd;

	// if (m_game->m_input.keyPressed(App::KEY_SPACE))
	// {
	// 	moveType = !moveType;

	// 	if (moveType)
	// 	{
	// 		printf("Lerp movement\n");
	// 	}
	// 	else
	// 	{
	// 		printf("Damp movement\n");
	// 	}
	// }

	if (moveType)
	{
		// Lerp movement
		velocity.x = Math::lerp(velocity.x, targetVelocity.x,
								Math::abs(targetVelocity.x) > 0 ? accel : decel);
		velocity.y = Math::lerp(velocity.y, targetVelocity.y,
								Math::abs(targetVelocity.y) > 0 ? accel : decel);
	}
	else
	{
		// Damp movement
		constexpr float dMult = 1.0f;
		velocity.x = Math::damp(velocity.x, targetVelocity.x,
								Math::abs(targetVelocity.x) > 0 ? accelDamp : decelDamp, Time::deltaSeconds * dampDtMult);
		velocity.y = Math::damp(velocity.y, targetVelocity.y,
								Math::abs(targetVelocity.y) > 0 ? accelDamp : decelDamp, Time::deltaSeconds * dampDtMult);
	}

	// Bouncing animation
	if (inputAxis != Vec2f(0, 0))
	{
		self.player->walkBounce += Time::delta;
		self.player->walkRaise = Math::abs(Math::sin(self.player->walkBounce / walkBounceFreq) * walkBounceAmp);
	}
	else
	{
		self.player->walkBounce = 0.0f;
		self.player->walkRaise = Math::damp(self.player->walkRaise, 0.0f, 0.2f, Time::deltaSeconds * dampDtMult);

		// Original lerp
		// self.player->walkRaise = Math::lerp(self.player->walkRaise, 0.0f, 0.2f * Time::delta);
	}
	self.animator->offset.y = -self.player->walkRaise;
}

void UpdatePlayer::handleGrappling(struct PlayerCommon::PlayerEntity &self)
{
	Vec2i zOffsetPos = self.transform->pos + grapplePlayerOffset - Vec2i(0, self.transform->z);

	// Check for line of sight collision with any grapplable entities
	bool canGrapple = false;
	{
		float closestLength = -1;
		int closestZ = 0;
		Vec2i closestPoint;

		Linei los = Linei(zOffsetPos, m_game->m_input.mousePos());
		for (const auto &gEnt : *m_grapplableEntities)
		{
			auto &gCollider = m_scene->getComponent<Collider2D>(gEnt);

			Vec2i intersectionPoint;
			if (los.intersectsClosest(self.transform->pos, gCollider.rect, &intersectionPoint))
			{
				canGrapple = true;

				// Get the closest intersection point out of all the grapplable colliders we've checked...
				float checkLength = (self.transform->pos - intersectionPoint).lengthSquared();
				if (closestLength < 0 || checkLength < closestLength)
				{
					closestLength = checkLength;
					closestPoint = intersectionPoint;

					if (m_scene->hasComponent<Transform2D>(gEnt))
					{
						auto &transform = m_scene->getComponent<Transform2D>(gEnt);
						closestZ = transform.z;
					}
				}
			}
		}

		self.player->laserLine = los;
		self.player->grappleTargetPoint = closestPoint;
		self.player->grappleTargetZ = closestZ;
	}

	if (!self.player->grappleTargeting && canGrapple)
	{
		Audio::play(Content::sndLockOn);
	}

	self.player->grappleTargeting = canGrapple;

	// If we press the grapple button, we initiate a grapple
	if (canGrapple && m_game->m_input.mButtonPressed(bGrapple))
	{
		shootGrapple(self);
	}
}

void UpdatePlayer::shootGrapple(PlayerEntity &self)
{
	Vec2i zOffsetPos = self.transform->pos + grapplePlayerOffset - Vec2i(0, self.transform->z);

	// Initialize grapple line
	self.player->grappleLine = Linef(
		zOffsetPos,
		zOffsetPos);

	// Scale cursor
	self.player->cursorScale = 4.5f;

	// Sound
	Audio::play(Content::sndGrappleShoot, true);
	Audio::play(Content::sndGrappleExtend, true, true);

	Factory::smokeExplosion(m_scene, zOffsetPos, Math::randRangei(4, 8));

	switchState(m_scene, self, Player::State::ShotGrapple);
}

void UpdatePlayer::startReeling(PlayerEntity &self)
{
	// Get normalized grapple vector
	{
		Vec3f nrm = calcGrappleNormal(self);

		// X Y
		self.player->grappleNormal = Vec2f(nrm.x, nrm.y);

		// Z
		self.player->elevationNormal = nrm.z;

		// Figure out stating signs so we can prevent overshoot later
		self.player->grappleSigns = Vec3i(
			Math::sign(nrm.x),
			Math::sign(nrm.y),
			Math::sign(nrm.z));
	}

	// Reset our reeling speed
	self.player->reelSpeed = 0.0f;

	// Prevent being able to hold into a wall while we start reeling, causing an instant bounce
	self.mover->velocity = Vec2f(0, 0);

	// Setup callback so we bounce on collision with a solid
	// TODO: Also collide with enemies
	// TODO: Handle collision with other objects
	auto bounceCallback = [this, self](Entity ent)
	{
		auto _self = self;
		checkImpactCollision(_self, ent);
	};
	Mover2DCommon::setOnCollide(*self.mover, bounceCallback);

	// NOTE: There's a bug where the player won't stop grappling if their grappleNormal is
	// almost parallel to the wall they're grappling to
	// UPDATE: This is part of the same "overshooting" glitch

	// Sounds
	Audio::play(Content::sndGrappleImpact, true);
	Audio::play(Content::sndGrappleHook, true);
	Audio::play(Content::sndGrappleReel, true, true);

	Audio::stop(Content::sndGrappleExtend);

	// Smoke and shake
	Factory::smokeExplosion(m_scene, self.player->grappleLine.b, Math::randRangei(6, 12));
	m_scene->m_camera.shake(shakeAmount);

	switchState(m_scene, self, Player::State::GrappleReeling);
}

void UpdatePlayer::doGrappleBounce(PlayerEntity &self)
{
	// Setup time to exit this state
	self.player->stateTimer = bounceTime;

	// Give us bounce velocity
	self.mover->velocity = -self.player->grappleNormal * bounceSpd;

	// Sound
	Audio::play(Content::sndSlam, true);
	Audio::stop(Content::sndGrappleReel);

	// Smoke and shake
	// Factory::smokeExplosion(m_scene, self.player->grappleLine.b, Math::randRangei(6, 12));
	Factory::smokeExplosion(m_scene, self.transform->pos, Math::randRangei(6, 12));
	m_scene->m_camera.shake(shakeAmount);

	Factory::shockwaveEffect(m_scene, self.transform->pos);

	switchState(m_scene, self, Player::State::GrappleBounce);
}

void GS::UpdatePlayer::attachCeilingHook(PlayerCommon::PlayerEntity &self, Engine::Entity hookEnt)
{
	auto &uid = m_scene->getComponent<UID>(hookEnt);
	auto &animator = m_scene->getComponent<Animator>(hookEnt);
	auto &hookTransform = m_scene->getComponent<Transform2D>(hookEnt);

	self.player->ceilingHookUid = uid.id;

	animator.offset = self.mover->velocity.normalized() * ceilHookBounceOffset;
	self.animator->offset = self.mover->velocity.normalized() * ceilHookBounceOffset;
	animator.lerpOffsetFac = 0.2;
	self.animator->lerpOffsetFac = 0.2;

	self.mover->velocity = Vec2f(0, 0);
	self.transform->z = hookTransform.z;
	self.gravity->zVelocity = 0;

	Audio::play(Content::sndHookEffect);
	Factory::hookEffect(m_scene, TransformCommon::elevatedHeight(hookTransform));
	m_scene->m_camera.shake(shakeAmount);

	switchState(m_scene, self, Player::State::CeilingHook);
}

void UpdatePlayer::checkImpactCollision(PlayerCommon::PlayerEntity &self, Entity collidingEnt)
{
	// Check what we just collided with
	if (m_scene->hasComponent<CeilingHook>(collidingEnt))
	{
		attachCeilingHook(self, collidingEnt);
	}
	else if (m_scene->hasComponent<Enemy>(collidingEnt))
	{
		// doGrappleBounce(self);
	}
	else
	{
		doGrappleBounce(self);
	}
}

bool UpdatePlayer::checkReelingCollision(PlayerCommon::PlayerEntity &self)
{
	Recti offsetCollider = self.collider->rect;
	offsetCollider.y -= self.transform->z;

	for (auto const &ent : *m_grapplableEntities)
	{
		auto &collider = m_scene->getComponent<Collider2D>(ent);
		if (offsetCollider.overlaps(collider.rect))
		{
			checkImpactCollision(self, ent);
			return true;
		}
	}
	return false;
}

Vec3f UpdatePlayer::calcGrappleNormal(PlayerCommon::PlayerEntity &self)
{
	Vec3f tmp = Vec3f(self.transform->pos.x, self.transform->pos.y, self.transform->z);
	Vec3f tmp2 = Vec3f(self.player->grappleTargetPoint.x, self.player->grappleTargetPoint.y + self.player->grappleTargetZ, self.player->grappleTargetZ);
	return (tmp2 - tmp).normalized();
}

void GS::UpdatePlayer::checkEnemyCollision(PlayerCommon::PlayerEntity &self)
{
	UpdateEnemy *enemySys = m_scene->getSystem<UpdateEnemy>();
	if (enemySys->m_entities.size() < 0)
	{
		return;
	}

	if (self.player->state == Player::State::Main ||
		self.player->state == Player::State::ShotGrapple ||
		self.player->state == Player::State::CeilingHook)
	{
		// Touching an enemy kills us
		for (auto const &ent : enemySys->m_entities)
		{
			auto &collider = m_scene->getComponent<Collider2D>(ent);
			if (self.collider->rect.overlaps(collider.rect))
			{
				// TODO:
				// Die
				die(self);
				
				return;
			}
		}
	}
	else
	{
		// Touching an enemy kills the enemy
		for (auto const &ent : enemySys->m_entities)
		{
			auto &collider = m_scene->getComponent<Collider2D>(ent);
			if (self.collider->rect.overlaps(collider.rect))
			{
				// Kill enemy
				EnemyCommon::kill(m_scene, ent);
				self.player->enemiesKilled++;
			}
		}
	}
}

void GS::UpdatePlayer::die(PlayerCommon::PlayerEntity &self)
{
	Factory::shockwaveEffect(m_scene, self.transform->pos);
	Factory::smokeExplosion(m_scene, self.transform->pos, 50);
	Audio::play(Content::sndDeathPlayer);
	switchState(m_scene, self, Player::State::Dead);
}