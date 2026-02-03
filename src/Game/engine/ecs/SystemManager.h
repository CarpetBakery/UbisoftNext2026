#ifndef _SYSTEM_MANAGER_H
#define _SYSTEM_MANAGER_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>

#include "Types.h"
#include "System.h"
#include <engine/DebugConsole.h>

namespace Engine
{
    class Scene;

    class SystemManager
    {
    private:
        // SystemVector is sorted based on the system's 'priority' value.
        // If two systems have ths same priority, they will have an unknown update order
        using SystemVector = std::vector<std::shared_ptr<System>>;

        // Map from system type string pointer to a signature
        std::unordered_map<const char *, Signature> mSignatures{};

        // Map from system type string pointer to a system pointer
        std::unordered_map<const char *, std::shared_ptr<System>> mSystems{};

        // An array mapping System::Type to SystemVector's
        std::array<SystemVector, static_cast<size_t>(System::Type::Count)> mSystemUpdateList;

        // Entities that have been queued for destruction
        std::queue<Entity> destroyQueue{};
        std::unordered_set<Entity> destroySet{};

        void updateSystems(const SystemVector &systemVector);

    public:
        template <typename T>
    std::shared_ptr<T> registerSystem(Scene *scene)
        {
            // NOTE: I'm fine with not creating a "safe" version of this function like in ComponentManager
            // because I want to explicitly register all systems in a scene

            const char *name = typeid(T).name();
            LB_ASSERT(!mSystems.count(name), "Registering system more than once.");

            // Create a pointer to the system and return it so it can be used externally
            std::shared_ptr<T> system = std::make_shared<T>();
            mSystems.insert({name, system});

            // -- Set the system up --
            {
                // Pass dependencies
                system->m_game = scene->getGame();
                system->m_scene = scene;

                // Run system's init (to set up type and priority before the next step)
                system->init();

                // Setup signature using private member var that is never ever used again
                scene->setSystemSignature<T>(system->m_setupSignature);
            }

            // Insert into our update list
            {
                SystemVector &updateList = mSystemUpdateList.at(static_cast<size_t>(system->m_type));
                updateList.push_back(system);

                // Immediately sort based on priority
                std::sort(updateList.begin(), updateList.end(), [](const auto &ls, const auto &rs)
                          { return ls->m_priority < rs->m_priority; });
            }

            return system;
        }

        // Update single system type
        void updateType(System::Type type)
        {
            size_t nType = static_cast<size_t>(type);
            SystemVector systemVector = mSystemUpdateList.at(nType);
            updateSystems(systemVector);
        }

        // Update systems within a range of system types
        void updateTypeRange(System::Type typeStart, System::Type typeEnd)
        {
            int start = static_cast<size_t>(typeStart);
            int end = static_cast<size_t>(typeEnd);

            // size_t typeCount = static_cast<size_t>(System::Type::Count);
            for (size_t type = start; type < end; type++)
            {
                SystemVector systemVector = mSystemUpdateList.at(type);
                updateSystems(systemVector);
            }
        }

        // Update relevant systems
        void update()
        {
            updateTypeRange(static_cast<System::Type>(0), System::Type::PreDraw);
        }

        // Draw relevant systems
        void draw()
        {
            updateTypeRange(System::Type::PreDraw, System::Type::Count);
        }

        template <typename T>
        void setSignature(const Signature &signature)
        {
            const char *name = typeid(T).name();
            LB_ASSERT(mSystems.count(name), "System used before registered.");

            // Set the signature for this system
            mSignatures.insert({name, signature});
        }

        void entityDestroyed(const Entity entity)
        {
            // Erase a destroyed entity from all system lists
            // m_entities is a set, so no check needed
            for (auto const &pair : mSystems)
            {
                auto const &system = pair.second;
                
                if (system->m_entities.count(entity))
                {
                    // Call entity removed func
                    system->entityRemoved(entity);
                }
            
                system->m_entities.erase(entity);
            }
        }

        void entitySignatureChanged(const Entity entity, const Signature &entitySignature)
        {
            // Notify each system that an entity's signature changed
            for (auto const &pair : mSystems)
            {
                auto const &type = pair.first;
                auto const &system = pair.second;
                auto const &systemSignature = mSignatures[type];

                // Entity signature matches system signature - insert into set
                if ((entitySignature & systemSignature) == systemSignature)
                {
                    system->m_entities.insert(entity);

                    // Call entityAdded func
                    system->entityAdded(entity);
                }
                else
                {   
                    // Entity signature does not match system signature - erase from set
                    system->m_entities.erase(entity);
                }
            }
        }

        void queueDestroy(const Entity entity)
        {
            if (destroySet.count(entity) <= 0)
            {
                destroyQueue.push(entity);
                destroySet.insert(entity);
            }
        }

        const SystemVector &debugGetSystemUpdateList(System::Type type) const
        {
            return mSystemUpdateList.at(static_cast<int>(type));
        }

        template <class T>
        T *getSystem()
        {
            const char *name = typeid(T).name();
            LB_ASSERT(mSystems.count(name), "System was never registered.");
            
            return static_cast<T*>(mSystems.at(name).get());
        }
    };
}

#endif // _SYSTEM_MANAGER_H