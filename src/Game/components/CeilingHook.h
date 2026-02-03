#ifndef _CEILING_HOOK_H
#define _CEILING_HOOK_H

#include <engine/ecs/System.h>

namespace GS
{
    struct CeilingHook
    {
        bool intro = false;

        bool startIntro = false;
        float introTimer = 0.0f;
    };
    
    
    class UpdateCeilingHook : public Engine::System
    {
    public:
        void init() override;
        void update() override;
    };
}

#endif // _CEILING_HOOK_H