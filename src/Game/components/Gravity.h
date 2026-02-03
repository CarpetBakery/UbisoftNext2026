#ifndef _GRAVITY_H
#define _GRAVITY_H

#include <engine/ecs/System.h>
#include <functional>

namespace GS
{
	struct Gravity
	{
		float intensity = 0.9f;

		float zVelocity = 0.0f;
		float peakVelocity = 0.0f;
		float remainder = 0.0f;

		// Percentage of velocity we retain after bouncing
		float bounciness = 0.7f;

		// Should we apply gravity?
		bool apply = true;

		// Called when we bounce
		std::function<void(void)> onBounce;
	};

	class UpdateGravity : public Engine::System
	{
	public:
		void init() override;
		void update() override;
	};
}

#endif // _GRAVITY_H