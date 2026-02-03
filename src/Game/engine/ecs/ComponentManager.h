#ifndef _COMPONENT_MANAGER_H
#define _COMPONENT_MANAGER_H

#include <unordered_map>
#include <memory>
#include "Types.h"
#include "ComponentArray.h"

namespace Engine
{
    class ComponentManager
    {
    private:
        // Map from type string to a component type
        std::unordered_map<const char *, ComponentType> mComponentTypes{};

        // Map from type string to a component array
        // TODO: Figure out why we need to have this on heap
        std::unordered_map<const char *, std::shared_ptr<IComponentArray>> mComponentArrays{};

        // The component type to be assigned to the next registered component - starting at 0
        ComponentType mNextComponentType{};

        // Debug array for storing component type names so we can print them in scene
        std::vector<std::string> mDebugComponentNames{};

        // Convenience function to get the statically casted pointer to the ComponentArray of type T
        template <typename T>
        std::shared_ptr<ComponentArray<T>> getComponentArray()
        {
            const char *name = typeid(T).name();
            LB_ASSERT(mComponentTypes.find(name) != mComponentTypes.end(), "Component not registered before use.");

            return std::static_pointer_cast<ComponentArray<T>>(mComponentArrays[name]);
        }

        template <typename T>
        void registerComponentSafe()
        {
            const char *name = typeid(T).name();
            if (!mComponentTypes.count(name))
            {
                registerComponent<T>();
            }
        }

    public:
        // Register a new component type
        template <typename T>
        void registerComponent()
        {
            const char *name = typeid(T).name();
            LB_ASSERT(mComponentTypes.find(name) == mComponentTypes.end(), "Registering component type more than once.");

            // Add this component type to the component type map
            mComponentTypes.insert({name, mNextComponentType});

            // Create a ComponentArray pointer and add it to the component arrays map
            mComponentArrays.insert({name, std::make_shared<ComponentArray<T>>()});

            // Increment value so that the next component registered will be different
            mNextComponentType++;

            // Insert into debug names list
            mDebugComponentNames.push_back(name);
        }

        template <typename T>
        ComponentType getComponentType()
        {
            const char *name = typeid(T).name();
            LB_ASSERT(mComponentTypes.find(name) != mComponentTypes.end(), "Component not registered before use.");

            return mComponentTypes[name];
        }

        // Add a component to an entity
        template <typename T>
        void add(const Entity entity, const T &component)
        {
            // Register component if it doesn't exist yet
            registerComponentSafe<T>();
            getComponentArray<T>()->insert(entity, component);
        }

        // Remove component from entity
        template <typename T>
        void remove(const Entity entity, const T &component)
        {
            registerComponentSafe<T>();
            getComponentArray<T>()->remove(entity);
        }

        // Add component to entity
        // Will trigger assert if component type was not previously registered
        template <typename T>
        void addUnsafe(const Entity entity, const T &component)
        {
            getComponentArray<T>()->insert(entity, component);
        }

        // Remove component from entity
        // Will trigger assert if component type was not previously registered
        template <typename T>
        void removeUnsafe(const Entity entity, const T &component)
        {
            getComponentArray<T>()->remove(entity);
        }

        // Get a component from an entity
        template <typename T>
        T &get(const Entity entity)
        {
            return getComponentArray<T>()->get(entity);
        }

        // Does this entity have this component?
        template <typename T>
        bool has(const Entity entity)
        {
            return getComponentArray<T>()->has(entity);
        }

        void entityDestroyed(const Entity entity)
        {
            // Notify each component array that an entity has been destroyed
            for (auto const &pair : mComponentArrays)
            {
                auto const &componentArray = pair.second;
                componentArray->entityDestroyed(entity);
            }
        }

        const std::vector<std::string>& debugGetRegisteredComponentNames() const
        {
            return mDebugComponentNames;
        }
    };
}

#endif // _COMPONENT_MANAGER_H