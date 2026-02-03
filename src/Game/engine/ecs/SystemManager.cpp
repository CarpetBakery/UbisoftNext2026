#include "SystemManager.h"

#include "Scene.h"

using namespace Engine;

void SystemManager::updateSystems(const SystemVector &systemVector)
{
    for (auto &system : systemVector)
    {
        destroySet.clear();

        system->update();

        // Remove any entities that have been queued for deletion
        while (!destroyQueue.empty())
        {
            Entity ent = destroyQueue.front();
            system->m_scene->destroyEntity(ent);
            destroyQueue.pop();
        }
        
        // printf("Alive entities: %d\n", system->m_scene->mEntityManager.livingEntityCount());
    }
}