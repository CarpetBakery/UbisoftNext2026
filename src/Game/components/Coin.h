#ifndef _COIN_H
#define _COIN_H

#include <engine/ecs/System.h>

namespace GS
{
	struct Coin
	{
		float animOffset = 0.0f;

		float destroytimer = 240.0f;
	};

	class UpdateCoin : public Engine::System
	{
	public:
		void init() override;
		void update() override;
	};
}

#endif // _COIN_H