#include "../MouseCollider.h"

#include <engine/Ecs.h>

using namespace Engine;

void UpdateMouseColliders::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<MouseCollider>());
    setup(-10, Type::Draw, sig);
}

void UpdateMouseColliders::update()
{
    for (auto const &ent : m_entities)
    {
        auto &mouseCollider = m_scene->getComponent<MouseCollider>(ent);
    }
}