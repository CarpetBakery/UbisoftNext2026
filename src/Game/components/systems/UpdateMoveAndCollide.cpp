#include <components/Mover2D.h>

#include <engine/Ecs.h>
#include <engine/Time.h>

#include <engine/components/Transform2D.h>
#include <engine/components/Collider2D.h>
#include <components/Solid.h>

using namespace Engine;
using namespace GS;

void UpdateMoveAndCollide2D::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Collider2D>());
	sig.set(m_scene->getComponentType<Mover2D>());
	setup(0, Type::Update, sig);
}

void UpdateMoveAndCollide2D::update()
{
	m_updateSolids = m_scene->getSystem<UpdateSolids>();

	for (const auto &ent : m_entities)
	{
		Mover2DCommon::MoverEntity self;
		self.ent = ent;
		self.transform = &m_scene->getComponent<Transform2D>(ent);
		self.collider = &m_scene->getComponent<Collider2D>(ent);
		self.mover = &m_scene->getComponent<Mover2D>(ent);

		Vec2f total = self.mover->remainder + (self.mover->velocity * Graphics::resMult) * Time::delta;
		Vec2i toMove = Vec2i(static_cast<int>(total.x), static_cast<int>(total.y));

		self.mover->remainder = total - toMove;

		moveAndCollideX(&self, toMove.x);
		moveAndCollideY(&self, toMove.y);
	}
}

void UpdateMoveAndCollide2D::moveAndCollideX(Mover2DCommon::MoverEntity *self, int amt)
{
	int sign = Math::sign(amt);
	Entity collidingEnt;
	while (amt != 0)
	{
		if (checkCollision(self, Vec2i(sign, 0), &collidingEnt))
		{
			self->mover->velocityBeforeCollision = self->mover->velocity;
			Mover2DCommon::stopX(self);

			if (self->mover->onCollideX)
			{
				self->mover->onCollideX(collidingEnt);
			}
			return;
		}

		amt -= sign;
		self->transform->pos.x += sign;
		self->collider->rect.x += sign;
	}
}

void UpdateMoveAndCollide2D::moveAndCollideY(Mover2DCommon::MoverEntity *self, int amt)
{
	int sign = Math::sign(amt);
	Entity collidingEnt;
	while (amt != 0)
	{
		if (checkCollision(self, Vec2i(0, sign), &collidingEnt))
		{
			self->mover->velocityBeforeCollision = self->mover->velocity;
			Mover2DCommon::stopY(self);

			if (self->mover->onCollideY)
			{
				self->mover->onCollideY(collidingEnt);
			}
			return;
		}

		amt -= sign;
		self->transform->pos.y += sign;
		self->collider->rect.y += sign;
	}
}

bool UpdateMoveAndCollide2D::checkCollision(Mover2DCommon::MoverEntity *self, Vec2i offset, Entity *collidingEntity)
{
	Recti offsetCollider = self->collider->rect;
	offsetCollider.x += offset.x;
	offsetCollider.y += offset.y;

	for (const auto &otherEnt : m_updateSolids->m_entities)
	{
		if (otherEnt == self->ent)
		{
			continue;
		}

		auto &otherCollider = m_scene->getComponent<Collider2D>(otherEnt);
		if (offsetCollider.overlaps(otherCollider.rect))
		{
			if (collidingEntity)
			{
				*collidingEntity = otherEnt;
			}
			return true;
		}
	}

	return false;
}