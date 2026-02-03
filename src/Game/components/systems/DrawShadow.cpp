#include <components/Shadow.h>

#include <engine/Ecs.h>

#include <engine/components/Transform2D.h>

#include <Content.h>

using namespace Engine;
using namespace GS;

void DrawShadow::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Shadow>());

	setup(-1, Type::Draw, sig);
}

void DrawShadow::update()
{
	for (auto const &ent : m_entities)
	{
		auto &transform = m_scene->getComponent<Transform2D>(ent);
		auto &shadow = m_scene->getComponent<Shadow>(ent);

		Content::sprShadow.SetOrigin(Sprite::Origin::Middle);
		Content::sprShadow.DrawExCam(transform.pos + shadow.offset);
	}
}