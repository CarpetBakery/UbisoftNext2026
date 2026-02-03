#include <components/Grapplable.h>

#include <engine/Ecs.h>

#include <engine/components/Collider2D.h>

using namespace Engine;
using namespace GS;

void UpdateGrapplable::init()
{
    Signature sig;
    
    sig.set(m_scene->getComponentType<Grapplable>());
    sig.set(m_scene->getComponentType<Collider2D>());

    setup(0, Type::Update, sig);
}

void UpdateGrapplable::update()
{}