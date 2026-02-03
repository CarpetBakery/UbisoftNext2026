#ifndef _GAMEPLAY_COMPONENTS_H
#define _GAMEPLAY_COMPONENTS_H

#include <engine/components/Transform2D.h>
#include <engine/components/Collider2D.h>
#include <engine/components/Animator.h>
#include <components/Mover2D.h>
#include <components/UID.h>
#include <components/Solid.h>
#include <components/Player.h>
#include <components/Grapplable.h>
#include <components/Particles2D.h>
#include <components/CameraController.h>
#include <components/Shadow.h>
#include <components/Coin.h>
#include <components/Gravity.h>
#include <components/CeilingHook.h>
#include <components/Transition.h>
#include <components/Enemy.h>
#include <components/EnemyGen.h>

#include <engine/ecs/Scene.h>

namespace GS
{
    // Register components and systems
    void registerGameplayComponents(Engine::Scene *scene);
}

#endif // _GAMEPLAY_COMPONENTS_H