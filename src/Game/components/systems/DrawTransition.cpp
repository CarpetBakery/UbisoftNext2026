#include <components/Transition.h>

#include <engine/Ecs.h>
#include <engine/Time.h>
#include <engine/EasingEquations.h>
#include <engine/Graphics.h>

using namespace Engine;
using namespace GS;

namespace
{
	constexpr float transitionTime = 0.2f;
}


void DrawTransition::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Transition>());
	setup(20, Type::Draw, sig);
}

void DrawTransition::update()
{
	float sw = m_game->getScreenWidth();
	float sh = m_game->getScreenHeight();

	for (auto const &ent : m_entities)
	{
		auto &transition = m_scene->getComponent<Transition>(ent);

		transition.time = Math::approach(transition.time, transitionTime, Time::deltaSeconds);

		Recti rect1 = Recti(Vec2i(0, 0), Vec2i(sw, sh / 2));
		Recti rect2 = Recti(Vec2i(0, 0), Vec2i(sw, sh / 2));

		float timeFac = transition.time / transitionTime;
		if (transition.type == 1)
		{
			rect1.y = Math::lerp(-sh / 2, 0, timeFac);
			rect2.y = Math::lerp(sh, sh/2, timeFac);
		}
		else
		{
			//rect1.y = cubic::in_out(transition.time, 0, -sh / 2, transitionTime);
			//rect2.y = cubic::in_out(transition.time, sh / 2, sh + (sh / 2), transitionTime);

			rect1.y = Math::lerp(0, -sh / 2, timeFac);
			rect2.y = Math::lerp(sh / 2, sh, timeFac);
		}

		Graphics::drawRectFilled(rect1, Color::black);
		Graphics::drawRectFilled(rect2, Color::black);

		if (transition.time >= transitionTime)
		{
			if (transition.callback)
			{
				transition.callback();
			}

			m_scene->queueDestroy(ent);
		}
	}
}
