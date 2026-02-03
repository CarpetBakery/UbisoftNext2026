#ifndef _ENTITY_MANAGER_H
#define _ENTITY_MANAGER_H

#include "Types.h"
#include <array>
#include <queue>
#include <engine/DebugConsole.h>

namespace Engine
{
    class EntityManager
    {
    private:
        // Queue of unused entity IDs
        std::queue<Entity> mAvailableEntries{};

        // Array of signatures where the index corresponds to the entity ID
        std::array<Signature, MAX_ENTITIES> mSignatures{};

        // Total living entities - used to keep limits on how many exist
        uint32_t mLivingEntityCount = 0;

    public:
        EntityManager()
        {
            // Initialize the queue with all possible entity IDs
            for (Entity entity = 0; entity < MAX_ENTITIES; entity++)
            {
                mAvailableEntries.push(entity);
            }
        }

        Entity createEntity()
        {
            LB_ASSERT(mLivingEntityCount < MAX_ENTITIES, "Too many entities in existence.");

            // Take an ID from the front of the queue
            Entity id = mAvailableEntries.front();
            mAvailableEntries.pop();
            mLivingEntityCount++;
            
            return id;
        }
        
        void destroyEntity(Entity entity)
        {
            LB_ASSERT(entity < MAX_ENTITIES, "Entity out of range");
            
            // Invalidate the destroyed entity's signature
            mSignatures[entity].reset();
            
            // Put the destroyed ID at the back of the queue
            mAvailableEntries.push(entity);
            mLivingEntityCount--;
        }

        void setSignature(Entity entity, Signature signature)
        {
            LB_ASSERT(entity < MAX_ENTITIES, "Entity out of range.");

            // Put this entity's signature into the array
            mSignatures[entity] = signature;
        }

        Signature getSignature(Entity entity)
        {
            LB_ASSERT(entity < MAX_ENTITIES, "Entities out of range.");

            // Get this entity's signature from the array
            return mSignatures[entity];
        }

        uint32_t livingEntityCount()
        {
            return mLivingEntityCount;
        }
    };
}

#endif // _ENTITY_MANAGER_H