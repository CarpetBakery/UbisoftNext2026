#ifndef _CAMERA_CONTROLLER_H
#define _CAMERA_CONTROLLER_H

#include <engine/ecs/System.h>
#include <engine/Spatial.h>

#include <components/UID.h>
#include <components/Player.h>

namespace GS
{
	struct CameraController
	{
		// Reference to player entity
		uint32_t playerId = -1;

		Engine::Vec2i screenBounds;
	};


	class UpdateCameraController : public Engine::System
	{
	private:
		UpdateUID *m_uidSys = nullptr;
		UpdatePlayer *m_playerSys = nullptr;
	public:
		void init() override;
		void update() override;
	};
}

#endif  _CAMERA_CONTROLLER_H