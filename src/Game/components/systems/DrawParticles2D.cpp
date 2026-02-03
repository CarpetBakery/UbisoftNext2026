#include <components/Particles2D.h>

#include <engine/Ecs.h>

using namespace Engine;
using namespace GS;

void DrawParticles2D::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Particles2D>());
	sig.set(m_scene->getComponentType<Transform2D>());

	// Particles draw on top of most other things
	setup(5, Type::Draw, sig);
}

void DrawParticles2D::update()
{
	for (auto const &ent : m_entities)
	{
		auto &pSystem = m_scene->getComponent<Particles2D>(ent);
		auto &particles = pSystem.particles;

		for (auto const &p : particles)
		{
			if (!p.spr)
			{
				continue;
			}

			// Draw this particle
			p.spr->SetOrigin(Sprite::Origin::Middle);
			p.spr->SetAngle(p.rotation);
			p.spr->DrawExCam(p.pos, p.scale, -1, p.color);
		}
	}
}