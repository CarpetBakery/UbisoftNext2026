#ifndef _PLAYER_H
#define _PLAYER_H

#include <engine/ecs/System.h>
#include <engine/ecs/Scene.h>

#include <engine/components/Transform2D.h>
#include <engine/components/Collider2D.h>
#include <engine/components/Animator.h>
#include <components/Mover2D.h>
#include <components/UID.h>
#include <components/Gravity.h>

namespace GS
{
	namespace PlayerCommon
	{
		struct PlayerEntity;
	}

	struct Player
	{
		enum class State
		{
			Main,
			ShotGrapple,
			GrappleReeling,
			GrappleBounce,
			CeilingHook,
			Dead,

			Count
		} state = State::Main;

		// Movement
		Engine::Vec2f targetVelocity = Engine::Vec2i(0, 0);

		// Grappling
		bool grappleTargeting = false;
		Engine::Linei laserLine = Engine::Linei(Engine::Vec2i(0, 0), Engine::Vec2i(0, 0));
		Engine::Vec2i grappleTargetPoint = Engine::Vec2i(0, 0);
		float grappleTargetZ = 0.0f;

		// The line that the rope follows
		Engine::Linef grappleLine = Engine::Linef(Engine::Vec2f(0, 0), Engine::Vec2f(0, 0));

		Engine::Vec3i grappleSigns = Engine::Vec3i(0, 0, 0);

		float reelSpeed = 0.0f;
		float stateTimer = 0.0f;

		// The angle of grappling that we've committed to
		// used to give us bounce dir on grapple impact
		Engine::Vec2f grappleNormal = Engine::Vec2f(0, 0);
		float elevationNormal = 0.0f;

		// Bouncy animation
		float walkBounce = 0.0f;
		float walkRaise = 0.0f;

		// Cursor
		float cursorScale = 1.0f;

		// Ceiling hook
		uint32_t ceilingHookUid;

		int enemiesKilled = 0;
		float timeSurvived = 0.0f;
		int coinsCollected = 0;
		bool intro = false;
	};

	class UpdatePlayer : public Engine::System
	{
	private:
		std::set<Engine::Entity> *m_grapplableEntities = nullptr;

	public:
		void init() override;
		void update() override;

		// -- States --
		// Main movement and shooting state
		void stMain(struct PlayerCommon::PlayerEntity &self);
		// State we're in right after shooting a grapple
		void stShotGrapple(struct PlayerCommon::PlayerEntity &self);
		// The player is being reeled to the grapple hook location
		void stGrappleReeling(struct PlayerCommon::PlayerEntity &self);
		// The player bounces away from the wall on impact
		void stGrappleBounce(struct PlayerCommon::PlayerEntity &self);
		// We're attached to a ceiling hook
		void stCeilingHook(struct PlayerCommon::PlayerEntity &self);

		void handleMovement(struct PlayerCommon::PlayerEntity &self, bool lockInput = false);
		void handleGrappling(struct PlayerCommon::PlayerEntity &self);

		void shootGrapple(struct PlayerCommon::PlayerEntity &self);
		void startReeling(struct PlayerCommon::PlayerEntity &self);
		void doGrappleBounce(struct PlayerCommon::PlayerEntity &self);
		void attachCeilingHook(struct PlayerCommon::PlayerEntity &self, Engine::Entity hookEnt);

		// Called when we've impacted with something we can collide with
		void checkImpactCollision(struct PlayerCommon::PlayerEntity &self, Engine::Entity collidingEnt);
		// Called every frame of reeling state
		bool checkReelingCollision(struct PlayerCommon::PlayerEntity &self);
		
		Engine::Vec3f calcGrappleNormal(struct PlayerCommon::PlayerEntity &self);

		void checkEnemyCollision(struct PlayerCommon::PlayerEntity &self);

		void die(struct PlayerCommon::PlayerEntity &self);
	};

	class DrawPlayer : public Engine::System
	{
	public:
		void init() override;
		void update() override;

		void drawGrappleTargeting(struct PlayerCommon::PlayerEntity &self);
		void drawGrappleHook(struct PlayerCommon::PlayerEntity &self);
	};

	namespace PlayerCommon
	{
		struct PlayerEntity
		{
			Engine::Entity ent;
			Transform2D *transform = nullptr;
			Collider2D *collider = nullptr;
			Animator *animator = nullptr;
			Mover2D *mover = nullptr;
			Player *player = nullptr;
			UID *uid = nullptr;
			Gravity *gravity = nullptr;
		};

		PlayerEntity getComponents(Engine::Scene *scene, Engine::Entity ent);
		void switchState(Engine::Scene *scene, PlayerEntity &self, Player::State newState);
	}
}

#endif // _PLAYER_H