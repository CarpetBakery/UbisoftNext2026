#include <components/Player.h>

#include <engine/Ecs.h>
#include <engine/Audio.h>

#include <Content.h>

using namespace Engine;
using namespace GS;

PlayerCommon::PlayerEntity PlayerCommon::getComponents(Scene *scene, Entity ent)
{
	PlayerEntity self;
	self.transform = &scene->getComponent<Transform2D>(ent);
	self.collider = &scene->getComponent<Collider2D>(ent);
	self.animator = &scene->getComponent<Animator>(ent);
	self.mover = &scene->getComponent<Mover2D>(ent);
	self.player = &scene->getComponent<Player>(ent);
	self.uid = &scene->getComponent<UID>(ent);
	self.gravity = &scene->getComponent<Gravity>(ent);

	self.ent = ent;

	return self;
}

void PlayerCommon::switchState(Scene *scene, PlayerEntity &self, Player::State newState)
{
	// Behavior when leaving state
	switch (self.player->state)
	{
	case Player::State::Main:
		self.player->grappleTargeting = false;
		break;

	case Player::State::ShotGrapple:
		Audio::stop(Content::sndGrappleExtend);
		break;

	case Player::State::GrappleReeling:
		// Reset function callback
		Mover2DCommon::setOnCollide(*self.mover, nullptr);
		Audio::stop(Content::sndGrappleReel);

		// Turn gravity back on
		self.gravity->apply = true;
		break;

	case Player::State::CeilingHook:
		// Turn gravity back on
		self.gravity->apply = true;
		break;
	}

	// Behavior when entering state
	switch (newState)
	{
	case Player::State::GrappleReeling:
		// Turn off gravity
		self.gravity->apply = false;
		break;

	case Player::State::CeilingHook:
		self.gravity->apply = false;
		break;
	}

	self.player->state = newState;
}
