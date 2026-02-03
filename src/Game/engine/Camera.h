#ifndef _CAMERA_H
#define _CAMERA_H

#include <engine/Spatial.h>

namespace Engine
{
	class Game;

	class Camera
	{
		friend class Game;
	private:
		Vec2i m_shakeOffset = Vec2i(0, 0);

	public:
		Vec2i m_pos = Vec2i(0, 0);
		Vec2f m_scale = Vec2f(1.0f, 1.0f);

		Vec2i m_size = Vec2i(0, 0);

		float m_shakeAmp = 0.0f;

		Recti getRect() const;
		void shake(float amt);
	};
}

#endif // _CAMERA_H