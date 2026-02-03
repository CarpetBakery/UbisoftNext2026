#ifndef _MOUSE_COLLIDER_H
#define _MOUSE_COLLIDER_H

#include <engine/Spatial.h>
#include <engine/ecs/System.h>

#include <vector>

// Check for collision with mouse
struct MouseCollider
{
    Engine::Recti rect;
    
    // Have some sort of "region" system so I can detect
    // dragging on an edge
    // std::vector<Engine::Recti> regions;
};

class UpdateMouseColliders : public Engine::System
{
public:
    void init() override;
    void update() override;
};

class DrawMouseColliders : public Engine::System
{
public:
    void init() override;
    void update() override;
};

#endif // _MOUSE_COLLIDER_H