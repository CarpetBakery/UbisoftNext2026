#ifndef _COMPONENT_ARRAY_H
#define _COMPONENT_ARRAY_H

#include "Types.h"
#include <array>
#include <unordered_map>
#include <engine/DebugConsole.h>

namespace Engine
{
    class IComponentArray
    {
    public:
        virtual ~IComponentArray() = default;
        virtual void entityDestroyed(Entity entity) = 0;
    };

    // We'll have one of these for each type of component
    template <typename T>
    class ComponentArray : public IComponentArray
    {
    private:
        // Packed array of components
        std::array<T, MAX_ENTITIES> mData;
        
        // Map from an entity ID to an array index
        // Given an entity, what index of the packed array contains its component?
        std::unordered_map<Entity, size_t> mEntityToIndex;
        
        // Map from an array index to an entity ID
        // Given an index of the packed array, what entity is it associated with?
        std::unordered_map<size_t, Entity> mIndexToEntity;

        // Total size of valid entries in array
        size_t mSize;
        
    public:
        // Insert a new component into the array
        void insert(const Entity entity, const T &component)
        {
            LB_ASSERT(mEntityToIndex.find(entity) == mEntityToIndex.end(), "Component added to entity more than once");
            
            // Put new entry at end and update the maps
            size_t newIndex = mSize;
            
            mEntityToIndex[entity] = newIndex;
            mIndexToEntity[newIndex] = entity;
            
            mData[newIndex] = component;

            mSize++;
        }

        // Remove a component from the array
        void remove(const Entity entity)
        {
            LB_ASSERT(mEntityToIndex.find(entity) != mEntityToIndex.end(), "Removing non-existent component.");

            size_t indexOfRemovedEntity = mEntityToIndex[entity];
            size_t indexLast = mSize - 1;
            Entity entityLast = mIndexToEntity[indexLast];

            // Copy component data from end into removed index
            mData[indexOfRemovedEntity] = mData[indexLast];

            // Given entity, return index to opened spot
            mEntityToIndex[entityLast] = indexOfRemovedEntity;            
            // Given index, return moved entity
            mIndexToEntity[indexOfRemovedEntity] = entityLast;

            mEntityToIndex.erase(entity);
            mIndexToEntity.erase(indexLast);
            mSize--;
        }

        // Get component data
        T &get(const Entity entity)
        {
            LB_ASSERT(mEntityToIndex.find(entity) != mEntityToIndex.end(), "Retrieving non-existent component.");
            return mData[mEntityToIndex[entity]];
        }

        // Check if entity has component
        bool has(const Entity entity)
        {
            return mEntityToIndex.find(entity) != mEntityToIndex.end(); 
        }

        void entityDestroyed(Entity entity) override
        {
            if (mEntityToIndex.find(entity) != mEntityToIndex.end())
            {
                // Remove entity's component if it existed
                remove(entity);
            }
        }

        const size_t size() const
        {
            return mSize;
        }
    };
}

#endif // _COMPONENT_ARRAY_H