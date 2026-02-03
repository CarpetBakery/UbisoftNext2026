#include <components/Enemy.h>

#include <engine/Ecs.h>
#include <engine/Time.h>
#include <GameplayComponents.h>

using namespace Engine;
using namespace GS;

namespace
{
	// constexpr float moveSpd = 4.0f;
	constexpr float turnSpd = 0.2f;
}

void UpdateEnemy::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Enemy>());
	sig.set(m_scene->getComponentType<Animator>());
	sig.set(m_scene->getComponentType<Collider2D>());
	sig.set(m_scene->getComponentType<Transform2D>());
	//sig.set(m_scene->getComponentType<Solid>());
	sig.set(m_scene->getComponentType<Grapplable>());
	sig.set(m_scene->getComponentType<Shadow>());

	setup(0, Type::Update, sig);
}

void UpdateEnemy::update()
{
	UpdatePlayer *playerSys = m_scene->getSystem<UpdatePlayer>();

	for (auto const &ent : m_entities)
	{
		auto &enemy = m_scene->getComponent<Enemy>(ent);
		auto &animator = m_scene->getComponent<Animator>(ent);
		auto &collider = m_scene->getComponent<Collider2D>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);
		auto &grapplable = m_scene->getComponent<Grapplable>(ent);
		auto &shadow = m_scene->getComponent<Shadow>(ent);

		animator.offset.y = Math::sin((Time::seconds * 8.0f) -enemy.spawnTime) * 8.0;
		animator.depth = transform.pos.y;

		if (playerSys->m_entities.size() <= 0)
		{
			continue;
		}

		Entity playerEnt = *playerSys->m_entities.begin();
		auto &playerTransform = m_scene->getComponent<Transform2D>(playerEnt);

		// Move toward the player
		Vec2f nrm = (playerTransform.pos - transform.pos).normalized();
		//transform.pos += nrm * enemy.moveSpd * Time::delta;
		transform.pos.x += nrm.x * enemy.moveSpd * Time::delta;
		transform.pos.y += nrm.y * enemy.moveSpd * Time::delta;

		collider.rect.x += nrm.x * enemy.moveSpd * Time::delta;
		collider.rect.y += nrm.y * enemy.moveSpd * Time::delta;

		if (nrm.x < 0)
		{
			transform.scale.x = Math::damp(transform.scale.x, -1, turnSpd, Time::deltaSeconds * 10.0f);
		}
		else
		{
			transform.scale.x = Math::damp(transform.scale.x, 1, turnSpd, Time::deltaSeconds * 10.0f);
		}
	}
}