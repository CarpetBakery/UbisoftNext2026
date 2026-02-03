#ifndef _GRAPPLABLE_H
#define _GRAPPLABLE_H

#include <engine/ecs/System.h>

namespace GS
{
    // An object that can be grappled
    struct Grapplable
    {};
    
    class UpdateGrapplable : public Engine::System
    {
    public:
        void init() override;
        void update() override;
    };
}


#endif // _GRAPPLABLE_H