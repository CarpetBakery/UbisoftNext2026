#ifndef _SOLID_H
#define _SOLID_H

#include <engine/ecs/System.h>

namespace GS
{
    // An unmoving solid object that movers will collide with
    struct Solid
    {};
    
    class UpdateSolids : public Engine::System
    {
    public:
        void init() override;
        // void update() override;
    };
}

#endif // _SOLID_H
