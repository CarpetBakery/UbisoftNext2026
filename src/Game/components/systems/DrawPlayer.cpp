#include <components/Player.h>

#include <engine/Ecs.h>
#include <engine/Time.h>

#include <Content.h>

using namespace Engine;
using namespace GS;
using namespace PlayerCommon;

namespace
{
	constexpr Vec2f defaultArrowScale = Vec2f(3.0f, 3.0f);

	bool drewCursor = false;
	constexpr float cursorSpinSpd = 8.0f;
	constexpr Color blueCol = Color(57, 132, 255);
}

void DrawPlayer::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Collider2D>());
	sig.set(m_scene->getComponentType<Animator>());
	sig.set(m_scene->getComponentType<Mover2D>());
	sig.set(m_scene->getComponentType<Player>());
	sig.set(m_scene->getComponentType<UID>());

	setup(5, Type::Draw, sig);
}

void DrawPlayer::update()
{
	drewCursor = false;
	for (const auto &ent : m_entities)
	{
		PlayerEntity self = getComponents(m_scene, ent);

		std::string surviveStr = "Time survived: " + std::to_string(self.player->timeSurvived) + " seconds"; 
		std::string enemiesKilledStr = "Enemies killed: " + std::to_string(self.player->enemiesKilled);
			
		
		switch (self.player->state)
		{
		case Player::State::Main:
			drawGrappleTargeting(self);
			break;

		case Player::State::ShotGrapple:
			drawGrappleHook(self);
			break;

		case Player::State::GrappleReeling:
			drawGrappleHook(self);
			break;

		case Player::State::Dead:
		// Graphics::drawRectFilled(Recti(Vec2i(0, 0), m_game->getScreenSize()), Color::fromFloat(0, 0, 0, 0.5));
			Graphics::drawText(Vec2i(460, 400), surviveStr, Color::white, GLUT_BITMAP_TIMES_ROMAN_24);
			Graphics::drawText(Vec2i(460, 440), enemiesKilledStr, Color::white, GLUT_BITMAP_TIMES_ROMAN_24);
			Graphics::drawText(Vec2i(460, 480), "PRESS 'R' TO RESTART", Color::white, GLUT_BITMAP_TIMES_ROMAN_24);
			self.transform->rotation += Time::delta * 10.0f;
			continue;
		}

		// Draw cursor
		if (!drewCursor)
		{
			drewCursor = true;

			// Inner cursor
			Content::sprCursorInner.SetAngle(Time::seconds * cursorSpinSpd);
			Content::sprCursorInner.SetOrigin(Sprite::Origin::Middle);

			// Outer cursor
			Vec2i outerPos = m_game->m_input.mousePos();
			Content::sprCursorOuter.SetAngle(Time::seconds * cursorSpinSpd);
			Content::sprCursorOuter.SetOrigin(Sprite::Origin::Middle);

			Vec2f scale = Vec2f(self.player->cursorScale, self.player->cursorScale);
			Color col = Color::white;

			if (self.player->grappleTargeting)
			{
				outerPos = self.player->grappleTargetPoint;
				col = blueCol;
			}
			else
			{
				Content::sprCursorOuter.DrawExCam(outerPos, scale, -1, col);
			}
			Content::sprCursorInner.DrawExCam(m_game->m_input.mousePos(), scale, -1, col);


			if (self.player->intro)
			{
				continue;
			}
			
			// Draw hud
			Graphics::drawText(Vec2i(10, 20), surviveStr, Color::white, GLUT_BITMAP_9_BY_15);
			Graphics::drawText(Vec2i(10, 40), enemiesKilledStr, Color::white, GLUT_BITMAP_9_BY_15);
		}
	}
}

void DrawPlayer::drawGrappleTargeting(PlayerEntity &self)
{
	if (!self.player->grappleTargeting)
	{
		return;
	}

	// -- Draw a line between the player and the mouse --
	Graphics::drawLineCam(m_scene,
						  self.player->laserLine.a,
						  self.player->grappleTargetPoint,
						  Color::red);


	
	Content::sprCursorOuter.DrawExCam(self.player->grappleTargetPoint, Vec2f(self.player->cursorScale, self.player->cursorScale), -1, blueCol);
	
	
	// OLD
	// Graphics::drawLineCam(m_scene,
	// 					  self.player->laserLine.a,
	// 					  self.player->laserLine.b,
	// 					  Color::red);
	// constexpr int rSize = 24;
	// Recti rect = Recti(self.player->grappleTargetPoint - Vec2i(rSize, rSize) / 2,
	// 				   Vec2i(rSize, rSize));
	// Graphics::drawRectCam(m_scene, rect, Color::blue);
}

void DrawPlayer::drawGrappleHook(PlayerEntity &self)
{
	float angle = self.player->grappleLine.angle();
	float length = self.player->grappleLine.length();

	// Draw rope
	Content::sprGrappleRope.SetOrigin(Sprite::Origin::Left);
	Content::sprGrappleRope.SetAngle(angle);
	Content::sprGrappleRope.DrawExCam(self.player->grappleLine.a, Vec2f(length, 1.0f));

	// Draw arrow
	Content::sprGrappleArrow.SetOrigin(Sprite::Origin::Left);
	Content::sprGrappleArrow.SetAngle(angle);
	Content::sprGrappleArrow.DrawExCam(self.player->grappleLine.b, defaultArrowScale);
}
