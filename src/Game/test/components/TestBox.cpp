#include "TestBox.h"

#include <engine/Ecs.h>

#include <engine/components/Transform2D.h>
#include <components/Mover2D.h>
#include <engine/components/Collider2D.h>

using namespace Engine;
using namespace GS;

namespace
{
	constexpr float spd = 400.0f;
}

void UpdateTestBox::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<TestBox>());
	sig.set(m_scene->getComponentType<Mover2D>());
	sig.set(m_scene->getComponentType<Transform2D>());

	setup(0, System::Type::Draw, sig);
}

void UpdateTestBox::update()
{
	for (auto const &ent : m_entities)
	{
		auto &mover = m_scene->getComponent<Mover2D>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);

		mover.velocity.x = static_cast<int>(m_game->m_input.key(App::KEY_RIGHT)) - static_cast<int>(m_game->m_input.key(App::KEY_LEFT));
		mover.velocity.x *= spd;

		mover.velocity.y = static_cast<int>(m_game->m_input.key(App::KEY_DOWN)) - static_cast<int>(m_game->m_input.key(App::KEY_UP));
		mover.velocity.y *= spd;

		Graphics::drawRect(Recti(transform.pos, Vec2i(20, 20)), Color::blue);
	}
}

Entity Factory::createTestBox(Scene *scene, const Recti &rect)
{
	Entity ent = scene->createEntity();

	Transform2D transform;
	transform.pos = rect.position();

	Collider2D collider;
	collider.rect = rect;

	Mover2D mover;

	TestBox testBox;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, collider);
	scene->addComponent(ent, mover);
	scene->addComponent(ent, testBox);

	return ent;
}

