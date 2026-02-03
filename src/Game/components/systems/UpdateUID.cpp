#include <components/UID.h>

#include <engine/Ecs.h>

using namespace Engine;
using namespace GS;

void UpdateUID::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<UID>());
    setup(0, Type::PreUpdate, sig);
}

void UpdateUID::entityAdded(Entity const &ent)
{
    auto &guid = m_scene->getComponent<UID>(ent);
    guid.id = m_id++;

    m_uidMap.insert({guid.id, ent});
}

void UpdateUID::entityRemoved(Engine::Entity const &ent)
{
    // Check if I need to move this before or after the entity is 
    // erased from system's entity list
    // TODO:

    auto &guid = m_scene->getComponent<UID>(ent);
    m_uidMap.erase(guid.id);
}

bool UpdateUID::entityExists(const uint32_t uid)
{
    return m_uidMap.count(uid) != 0;
}

Engine::Entity UpdateUID::getEntity(const uint32_t uid)
{
    LB_ASSERT(entityExists(uid), "Entity with that uid doesn't exist.");
    return m_uidMap.at(uid);
}