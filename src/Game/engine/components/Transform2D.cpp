#include "Transform2D.h"

using namespace Engine;

Engine::Vec2i TransformCommon::elevatedHeight(Transform2D &transform)
{
    return transform.pos - Vec2i(0, transform.z);
}