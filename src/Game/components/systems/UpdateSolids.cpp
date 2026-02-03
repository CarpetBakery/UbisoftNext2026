#include <components/Solid.h>

#include <engine/Ecs.h>

#include <engine/components/Collider2D.h>
#include <engine/components/Transform2D.h>

using namespace Engine;
using namespace GS;

// Mainly just here so we have a list of solid entities we can reference
// from other systems
void UpdateSolids::init()
{
    Signature sig;
    
    sig.set(m_scene->getComponentType<Solid>());
    sig.set(m_scene->getComponentType<Collider2D>());
    sig.set(m_scene->getComponentType<Transform2D>());

    setup(0, Type::Update, sig);
}