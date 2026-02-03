#ifndef _MOVER_2D_H
#define _MOVER_2D_H

#include <functional>

#include <engine/ecs/System.h>
#include <engine/Spatial.h>

#include <engine/components/Collider2D.h>
#include <engine/components/Transform2D.h>

// Thank you to Maddy Thorson for the idea behind this method
// https://maddythorson.medium.com/celeste-and-towerfall-physics-d24bd2ae0fc5

namespace GS
{
    namespace Mover2DCommon
    {
        struct MoverEntity;
    }

    struct Mover2D
    {
        Engine::Vec2f remainder;
        Engine::Vec2f velocity;

        // Set when a collision happens
        Engine::Vec2f velocityBeforeCollision;

        // Passes in the entity we collided with
        using CollideCallback = std::function<void(Engine::Entity ent)>;
        
        CollideCallback onCollideX;
        CollideCallback onCollideY;
    };

    // Update mover with collider
    class UpdateMoveAndCollide2D : public Engine::System
    {
    public:
        class UpdateSolids *m_updateSolids = nullptr;

        void init() override;
        void update() override;

    private:
        void moveAndCollideX(struct Mover2DCommon::MoverEntity *self, int amt);
        void moveAndCollideY(struct Mover2DCommon::MoverEntity *self, int amt);

        // Check if any collider in the scene is colliding with this
        bool checkCollision(struct Mover2DCommon::MoverEntity *self, Engine::Vec2i offset, Engine::Entity *collidingEntity = nullptr);
    };

    // Update mover without collider
    class UpdateMover2D : public Engine::System
    {
    public:
        void init() override;
        void update() override;
    };

    namespace Mover2DCommon
    {
        // Trying out new thing where I store current ent's components in a struct like this
        // and pass just this around instead of writing the same three args in every function...
        struct MoverEntity
        {
            Engine::Entity ent;
            Transform2D *transform = nullptr;
            Collider2D *collider = nullptr;
            Mover2D *mover = nullptr;
        };

        void stopX(struct Mover2DCommon::MoverEntity *self);
        void stopY(struct Mover2DCommon::MoverEntity *self);

        void stop(struct Mover2DCommon::MoverEntity *self);

        void setOnCollide(Mover2D &mover, Mover2D::CollideCallback callback);
        void setOnCollideX(Mover2D &mover, Mover2D::CollideCallback callback);
        void setOnCollideY(Mover2D &mover, Mover2D::CollideCallback callback);
    }
}

#endif // _MOVER_2D_H