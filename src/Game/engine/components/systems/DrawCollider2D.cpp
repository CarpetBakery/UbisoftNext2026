#include <engine/components/Collider2D.h>

#include <engine/Ecs.h>
#include <engine/Graphics.h>

using namespace Engine;

void DrawCollider2D::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<Collider2D>());
	setup(0, Type::PostDraw, sig);
}

void DrawCollider2D::update()
{
	// Draw a rectangle for every collider
	for (auto const &ent : m_entities)
	{
		auto &collider = m_scene->getComponent<Collider2D>(ent);
		Graphics::drawRectCam(m_scene, collider.rect, Color::red);
	}
}
