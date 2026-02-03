#ifndef _STATE_MACHINE_H
#define _STATE_MACHINE_H

#include <engine/ecs/System.h>

namespace GS
{
    struct StateMachine
    {
        // How far are we into the state?
        float progress = 0.0f;
    
        // How long does this state last for?
        float duration = 1.0f;
        
        // Increased by 1 every time progress equals duration
        uint16_t stateIndex = 0;
    };
}

#endif // _STATE_MACHINE_H