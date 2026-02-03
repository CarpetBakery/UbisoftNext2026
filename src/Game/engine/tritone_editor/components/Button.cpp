#include "Button.h"

#include <engine/Ecs.h>
#include <engine/Spatial.h>

#include "MouseCollider.h"
#include <engine/components/Transform2D.h>

using namespace Engine;

void setButtonBounds(Scene *scene, Entity ent, const Vec2i &pos, const Vec2i &size)
{
    auto &mouseCollider = scene->getComponent<MouseCollider>(ent);
    auto &transform = scene->getComponent<Transform2D>(ent);

    transform.pos = pos;

    if (size.x < 0 || size.y < 0)
    {
        mouseCollider.rect.x = pos.x;
        mouseCollider.rect.y = pos.y;
    }
    else
    {
        mouseCollider.rect = Recti(pos, size);
    }
}