#include <components/CameraController.h>

#include <engine/Ecs.h>
#include <engine/Graphics.h>
#include <engine/Time.h>

using namespace Engine;
using namespace GS;

namespace
{
	float maxDist = 96 * Graphics::resMult;
	//float maxDist = 96;
}

void UpdateCameraController::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<CameraController>());
	sig.set(m_scene->getComponentType<Transform2D>());

	setup(0, Type::Update, sig);
}

void UpdateCameraController::update()
{
	m_uidSys = m_scene->getSystem<UpdateUID>();
	m_playerSys = m_scene->getSystem<UpdatePlayer>();

	for (auto const &ent : m_entities)
	{
		auto &camController = m_scene->getComponent<CameraController>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);

		auto &camera = m_scene->m_camera;
		
		if (!m_uidSys->entityExists(camController.playerId))
		{
			// Find a new player
			if (m_playerSys->m_entities.size() > 0)
			{
				auto player = PlayerCommon::getComponents(m_scene, *m_playerSys->m_entities.begin());
				camController.playerId = player.uid->id;
			}
			else
			{
				// TOOD: Handle there being no player...
				continue;
			}
		}

		Entity playerEnt = m_uidSys->getEntity(camController.playerId);
		auto player = PlayerCommon::getComponents(m_scene, playerEnt);

		// Get direction from our position to mouse
		Linei mouseLine = Linei(transform.pos, m_game->m_input.mousePos());
		Vec2f dist = Vec2f(
			mouseLine.b.x - mouseLine.a.x, 
			mouseLine.b.y - mouseLine.a.y);

		Vec2f to;
		to.x = player.transform->pos.x + Math::clamp(dist.x, -maxDist, maxDist);
		to.y = player.transform->pos.y + Math::clamp(dist.y, -maxDist, maxDist);

		transform.pos.x += (to.x - transform.pos.x) / 20.0f * Time::delta;
		transform.pos.y += (to.y - transform.pos.y) / 20.0f * Time::delta;

		camera.m_pos.x = -(camera.m_size.x / 2.0f) + transform.pos.x;
		camera.m_pos.y = -(camera.m_size.y / 2.0f) + transform.pos.y;

		// -- Clamp within room here --
		camera.m_pos.x = Math::clamp(camera.m_pos.x, 0, camController.screenBounds.x - m_game->getScreenWidth());
		camera.m_pos.y = Math::clamp(camera.m_pos.y, 0, camController.screenBounds.y - m_game->getScreenHeight());
		

		// Shaking
		if (camera.m_shakeAmp > 0)
		{
			camera.m_pos.x += Math::randRange(-camera.m_shakeAmp, camera.m_shakeAmp);
			camera.m_pos.y += Math::randRange(-camera.m_shakeAmp, camera.m_shakeAmp);
			camera.m_shakeAmp = Math::approach(camera.m_shakeAmp, 0.0f, Time::delta * 4.0f);
		}
	}
}
