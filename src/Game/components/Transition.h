#ifndef _TRANSITION_H
#define _TRANSITION_H

#include <engine/ecs/System.h>
#include <functional>

namespace GS
{
	struct Transition
	{
		// 0 = in, 1 = out
		int type = 0;

		float time = 0.0f;

		std::function<void(void)> callback;
	};

	class DrawTransition : public Engine::System
	{
	public:
		void init() override;
		void update() override;
	};
}

#endif // _TRANSITION_H