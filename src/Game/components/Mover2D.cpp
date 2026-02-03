#include <components/Mover2D.h>

using namespace Engine;
using namespace GS;

void Mover2DCommon::stopX(Mover2DCommon::MoverEntity *self)
{
	self->mover->velocity.x = 0;
	self->mover->remainder.x = 0;
}

void Mover2DCommon::stopY(Mover2DCommon::MoverEntity *self)
{
	self->mover->velocity.y = 0;
	self->mover->remainder.y = 0;
}

void Mover2DCommon::stop(Mover2DCommon::MoverEntity *self)
{
	stopX(self);
	stopY(self);
}

void Mover2DCommon::setOnCollide(Mover2D &mover, Mover2D::CollideCallback callback)
{
	setOnCollideX(mover, callback);
	setOnCollideY(mover, callback);
}

void Mover2DCommon::setOnCollideX(Mover2D &mover, Mover2D::CollideCallback callback)
{
	mover.onCollideX = callback;
}

void Mover2DCommon::setOnCollideY(Mover2D &mover, Mover2D::CollideCallback callback)
{
	mover.onCollideY = callback;
}
