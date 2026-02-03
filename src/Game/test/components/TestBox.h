#ifndef _TEST_BOX_H
#define _TEST_BOX_H

#include <engine/ecs/Scene.h>
#include <engine/ecs/System.h>

namespace GS
{
	struct TestBox
	{};
	
	class UpdateTestBox : public Engine::System
	{
	public:
		void init() override;
		void update() override;
	};
	
	namespace Factory
	{
		Engine::Entity createTestBox(Engine::Scene *scene, const Engine::Recti &rect);
	}
}

#endif // _TEST_BOX_H