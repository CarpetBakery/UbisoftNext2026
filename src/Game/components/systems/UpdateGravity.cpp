#include <components/Gravity.h>

#include <engine/Ecs.h>
#include <engine/Time.h>

#include <engine/components/Transform2D.h>

#include <Factory.h>

using namespace Engine;
using namespace GS;

void UpdateGravity::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Gravity>());
	sig.set(m_scene->getComponentType<Transform2D>());

	setup(0, Type::Update, sig);
}

void UpdateGravity::update()
{
	for (auto const &ent : m_entities)
	{
		auto &gravity = m_scene->getComponent<Gravity>(ent);
		auto &transform = m_scene->getComponent <Transform2D>(ent);

		float total = gravity.remainder + gravity.zVelocity * Time::delta;
		int toMove = static_cast<int>(total);

		if (gravity.apply)
		{
			gravity.zVelocity += gravity.intensity * Time::delta;
		}
		
		gravity.remainder = total - toMove;
		transform.z -= toMove;
		// transform.z = Math::approach(transform.z, 0.0f, toMove);

		transform.z = Math::Max(transform.z, 0);
		
		// Bounce
		if (transform.z <= 0.0f && Math::abs(gravity.peakVelocity) > 2.0f)
		{
			gravity.peakVelocity *= gravity.bounciness;
			gravity.zVelocity = gravity.peakVelocity;

			if (gravity.onBounce)
			{
				gravity.onBounce();
			}
			else
			{
				Factory::smokeExplosion(m_scene, transform.pos, Math::randRangei(1, 2));
			}
		}
	}
}
