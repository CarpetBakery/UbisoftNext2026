#ifndef _TRANSFORM_2D_H
#define _TRANSFORM_2D_H

#include <engine/Spatial.h>

struct Transform2D
{
    // Position in pixels
    Engine::Vec2i pos = Engine::Vec2i(0, 0);

    // Optional fake z value
    int z = 0;

    // Scale (1.0 is 100% size)
    Engine::Vec2f scale = Engine::Vec2i(1, 1);

    // Rotation in degrees
    float rotation = 0.0f;
};

namespace TransformCommon
{
    Engine::Vec2i elevatedHeight(Transform2D &transform);
}

#endif // _TRANSFORM_2D_H