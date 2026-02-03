#ifndef _COLLIDER_2D_H
#define _COLLIDER_2D_H

#include <engine/Spatial.h>
#include <engine/ecs/System.h>

struct Collider2D
{
	Engine::Recti rect;

	// TODO: Need type?
};

class DrawCollider2D : public Engine::System
{
public:
	void init() override;
	void update() override;
};

#endif // _COLLIDER_2D_H