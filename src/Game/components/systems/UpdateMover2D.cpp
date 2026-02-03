#include <components/Mover2D.h>

#include <engine/Ecs.h>
#include <engine/Time.h>

#include <engine/components/Transform2D.h>
#include <engine/components/Collider2D.h>

using namespace Engine;
using namespace GS;

void UpdateMover2D::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Mover2D>());
	setup(0, Type::Update, sig);
}

void UpdateMover2D::update()
{
	for (auto const &ent : m_entities)
	{
		auto &mover = m_scene->getComponent<Mover2D>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);

		Vec2f total = mover.remainder + mover.velocity * Time::delta;
		Vec2i toMove = Vec2i(static_cast<int>(total.x), static_cast<int>(total.y));

		mover.remainder = total - toMove;

		transform.pos += toMove;
	}
}