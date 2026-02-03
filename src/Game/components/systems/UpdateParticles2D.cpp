#include <components/Particles2D.h>

#include <engine/Ecs.h>
#include <engine/Time.h>


using namespace Engine;
using namespace GS;

void UpdateParticles2D::init()
{
	Signature sig;

	sig.set(m_scene->getComponentType<Particles2D>());
	sig.set(m_scene->getComponentType<Transform2D>());

	setup(0, Type::Update, sig);
}

void UpdateParticles2D::entityAdded(Entity const &ent)
{
	auto &pSystem = m_scene->getComponent<Particles2D>(ent);
	auto &transform = m_scene->getComponent<Transform2D>(ent);

	if (pSystem.oneShot)
	{
		// Explosion of particles
		for (int i = 0; i < pSystem.particleCount; i++)
		{
			spawnParticle(pSystem, transform);
		}
	}
}

void UpdateParticles2D::update()
{
	for (auto const &ent : m_entities)
	{
		auto &pSystem = m_scene->getComponent<Particles2D>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);

		auto &particles = pSystem.particles;

		// Update particles
		for (auto &particle : particles)
		{
			float fac = particle.time / Math::Max(particle.lifeTime, 0.001f);
			particle.time += Time::deltaSeconds;

			// Move with velocity
			particle.velocity += (pSystem.gravityDir * pSystem.gravity) * Time::delta;
			particle.pos += particle.velocity * Time::delta * Graphics::resMult;

			// Animate rotation
			particle.rotation += Math::degToRad(particle.rotationSpd * Time::delta);

			// Animate scale
			particle.scale.x = Math::lerp(particle.scaleStart.x, particle.scaleEnd.x, fac);
			particle.scale.y = Math::lerp(particle.scaleStart.y, particle.scaleEnd.y, fac);

			// Animate color
			particle.color = particle.colorStart.lerp(particle.colorEnd, fac);
		}

		// Create particles on an interval
		if (!pSystem.oneShot && Time::onInterval(1.0f / pSystem.particleCount))
		{
			spawnParticle(pSystem, transform);
		}

		size_t len = particles.size();
		// Remove particles that are finished
		for (int i = len - 1; i >= 0; i--)
		{
			auto &particle = particles.at(i);
			if (particle.time >= particle.lifeTime)
			{
				particles.erase(particles.begin() + i);
			}
		}

		if (pSystem.oneShot)
		{
			if (particles.size() <= 0)
			{
				// Destroy this system if all our particles are dead
				m_scene->queueDestroy(ent);
			}
		}
		else
		{
			// If we don't have an infinite lifetime, advance our time
			if (pSystem.lifeTime > 0.0f)
			{
				pSystem.time += Time::deltaSeconds;
				if (pSystem.time >= pSystem.lifeTime)
				{
					// Our lifetime reached, we convert to a oneshot system
					// so we're destroyed when all particles are dead
					Particles2DCommon::destroy(pSystem);
				}
			}
		}

		// Temp
		if (m_game->m_input.keyPressed(App::KEY_D))
		{
			Particles2DCommon::destroy(pSystem);
		}
	}
}

void UpdateParticles2D::spawnParticle(Particles2D &pSystem, Transform2D &transform)
{
	pSystem.particles.emplace_back();
	Particles2D::Particle &p = pSystem.particles.back();

	// Lifetime
	p.lifeTime = Math::randRange(pSystem.particleLifeTimeMin, pSystem.particleLifeTimeMax);

	// Position/Velocity
	p.pos = transform.pos;
	p.velocity = Vec2f(
		Math::randRange(pSystem.velocityMin.x, pSystem.velocityMax.x),
		Math::randRange(pSystem.velocityMin.y, pSystem.velocityMax.y));

	// Sprite
	if (pSystem.spritePool.size() > 0)
	{
		int index = Math::randRangei(0, pSystem.spritePool.size() - 1);
		p.spr = pSystem.spritePool.at(index);
	}
	else
	{
		p.spr = pSystem.spritePool.at(0);
	}

	// Scale
	p.scaleStart = Vec2f(
		Math::randRange(pSystem.scaleStartMin, pSystem.scaleStartMax),
		Math::randRange(pSystem.scaleStartMin, pSystem.scaleStartMax));

	p.scaleEnd = Vec2f(
		Math::randRange(pSystem.scaleEndMin, pSystem.scaleEndMax),
		Math::randRange(pSystem.scaleEndMin, pSystem.scaleEndMax));

	// Rotation
	p.rotation = Math::randRange(pSystem.rotationMin, pSystem.rotationMax);
	p.rotationSpd = Math::randRange(pSystem.rotationSpeedMin, pSystem.rotationSpeedMax);

	// Color
	p.colorStart = pSystem.colorStart;
	p.colorEnd = pSystem.colorEnd;
}
