#include <components/Coin.h>

#include <engine/Ecs.h>
#include <engine/Time.h>

#include <engine/components/Transform2D.h>
#include <engine/components/Collider2D.h>
#include <engine/components/Animator.h>
#include <components/Mover2D.h>
#include <components/Gravity.h>

using namespace Engine;
using namespace GS;

namespace
{
	constexpr float flashThreshold = 120.0f;
}

void UpdateCoin::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Coin>());
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Collider2D>());
	sig.set(m_scene->getComponentType<Mover2D>());
	sig.set(m_scene->getComponentType<Animator>());
	sig.set(m_scene->getComponentType<Gravity>());

	setup(0, Type::Update, sig);
}

void UpdateCoin::update()
{
	for (auto const &ent : m_entities)
	{
		auto &coin = m_scene->getComponent<Coin>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);
		auto &collider = m_scene->getComponent<Collider2D>(ent);
		auto &mover = m_scene->getComponent<Mover2D>(ent);
		auto &gravity = m_scene->getComponent<Gravity>(ent);
		auto &animator = m_scene->getComponent<Animator>(ent);

		coin.destroytimer -= Time::delta;

		if (coin.destroytimer <= 0.0f)
		{
			m_scene->queueDestroy(ent);
			
		}

		if (coin.destroytimer < flashThreshold && Time::onInterval(0.05))
		{
			if (animator.color.a > 0)
			{
				animator.color.a = 0;
			}
			else
			{
				animator.color.a = 0xFF;
			}
		}

		// Animate scale
		transform.scale.x = Math::sin((Time::seconds - coin.animOffset) * 10.0f);
	}
}