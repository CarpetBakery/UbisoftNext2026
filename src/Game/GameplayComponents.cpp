#include "GameplayComponents.h"

using namespace Engine;
using namespace GS;

void GS::registerGameplayComponents(Scene *scene)
{
    // -- Components -- 
    scene->registerComponent<Animator>();
    scene->registerComponent<Mover2D>();
    scene->registerComponent<Transform2D>();
    scene->registerComponent<Collider2D>();
    scene->registerComponent<Solid>();
    scene->registerComponent<UID>();
    scene->registerComponent<Player>();
    scene->registerComponent<Grapplable>();
    scene->registerComponent<Particles2D>();
    scene->registerComponent<CameraController>();
    scene->registerComponent<Shadow>();
    scene->registerComponent<Coin>();
    scene->registerComponent<Gravity>();
    scene->registerComponent<CeilingHook>();
    scene->registerComponent<Transition>();
    scene->registerComponent<Enemy>();
    scene->registerComponent<EnemyGen>();

    // -- Systems --
    scene->registerSystem<UpdateAndDrawAnimator>();
    scene->registerSystem<UpdateMoveAndCollide2D>();
    scene->registerSystem<UpdateSolids>();
    scene->registerSystem<UpdateUID>();
    scene->registerSystem<UpdatePlayer>();
    scene->registerSystem<UpdateGrapplable>();
    scene->registerSystem<UpdateParticles2D>();
    scene->registerSystem<UpdateCameraController>();
    scene->registerSystem<UpdateCoin>();
    scene->registerSystem<UpdateGravity>();
    scene->registerSystem<UpdateCeilingHook>();
    scene->registerSystem<UpdateEnemy>();
    scene->registerSystem<UpdateEnemyGen>();
    
    scene->registerSystem<DrawPlayer>();
    scene->registerSystem<DrawParticles2D>();
    scene->registerSystem<DrawShadow>();
    scene->registerSystem<DrawTransition>();

    // DEBUG
    //scene->registerSystem<DrawCollider2D>();
}
