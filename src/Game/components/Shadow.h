#ifndef _SHADOW_H
#define _SHADOW_H

#include <engine/ecs/System.h>

namespace GS
{
	struct Shadow
	{
		Engine::Vec2i offset = Engine::Vec2i(0, 0);
	};

	class DrawShadow : public Engine::System
	{
	public:
		void init() override;
		void update() override;
	};
}

#endif // _SHADOW_H