#ifndef _SCENE_H
#define _SCENE_H

#include "Types.h"
#include "ComponentManager.h"
#include "EntityManager.h"
#include "SystemManager.h"

#include <engine/Math.h>
#include <engine/Time.h>

namespace Engine
{
	class Game;
	
	class Scene
	{
		friend class Engine::Game;

		// So it can access destroyEntity
		friend SystemManager;
		
	protected:
		ComponentManager mComponentManager;
		EntityManager mEntityManager;
		SystemManager mSystemManager;

		// Destroy and entity and its components
		void destroyEntity(const Entity entity)
		{
			mEntityManager.destroyEntity(entity);
			mSystemManager.entityDestroyed(entity);
			mComponentManager.entityDestroyed(entity);
		}

		// Debug restart the scene
		virtual void restart()
		{
			mComponentManager = ComponentManager();
			mEntityManager = EntityManager();
			mSystemManager = SystemManager();

			m_camera = Camera();
			m_camera.m_size = m_game->getScreenSize();
		}

		float m_hitStun = 0.0f;
		
	public:
		// Pointer to parent game class
		class Engine::Game *m_game = nullptr;

		// Camera object
		Camera m_camera;

		// -- Entity --
		// Create a new entity
		Entity createEntity()
		{
			return mEntityManager.createEntity();
		}

		// -- Component --
		// Register a component type
		template <typename T>
		void registerComponent()
		{
			mComponentManager.registerComponent<T>();
		}

		// Queue entity for destruction by the end of this update 
		void queueDestroy(const Entity entity)
		{
			mSystemManager.queueDestroy(entity);
		}

		// Create a component and add it to an entity
		template <typename T>
		void addComponent(const Entity entity, T component)
		{
			mComponentManager.add<T>(entity, component);

			auto signature = mEntityManager.getSignature(entity);
			signature.set(mComponentManager.getComponentType<T>(), true);
			mEntityManager.setSignature(entity, signature);

			mSystemManager.entitySignatureChanged(entity, signature);
		}

		// Remove a component from an entity
		template <typename T>
		void removeComponent(const Entity entity)
		{
			mComponentManager.remove<T>(entity);

			auto signature = mEntityManager.getSignature(entity);
			signature.set(mComponentManager.getComponentType<T>(), false);
			mEntityManager.setSignature(entity, signature);

			mSystemManager.entitySignatureChanged(entity, signature);
		}

		template <typename T>
		T &getComponent(const Entity entity)
		{
			return mComponentManager.get<T>(entity);
		}

		template <typename T>
		bool hasComponent(const Entity entity)
		{
			return mComponentManager.has<T>(entity);
		}

		template <typename T>
		ComponentType getComponentType()
		{
			return mComponentManager.getComponentType<T>();
		}

		// -- System --
		// Register a system to be used in this scene
		// also call its init function
		template <typename T>
		std::shared_ptr<T> registerSystem()
		{
			return mSystemManager.registerSystem<T>(this);
		}

		template <typename T>
		void setSystemSignature(const Signature &signature)
		{
			mSystemManager.setSignature<T>(signature);
		}

		template <typename T>
		T *getSystem()
		{
			return mSystemManager.getSystem<T>();
		}

		template <typename T>
		std::set<Entity> *getSystemEntities()
		{
			return &mSystemManager.getSystem<T>()->m_entities;
		}

		Engine::Game *getGame() const
		{
			return m_game;
		}

		std::string toString() const
		{
			std::string str = "";

			str += std::string(typeid(*this).name()) + " info:\n";

			// Print out all registered components
			str += "-- Components --\n";
			for (auto const& name : mComponentManager.debugGetRegisteredComponentNames())
			{
				str += "    " + name + "\n";
			}

			// Print out all registered systems and the order they execute in
			str += "\n-- Systems --\n";
			for (int i = 0; i < static_cast<int>(System::Type::Count); i++)
			{
				auto type = static_cast<System::Type>(i);
				auto const updateList = mSystemManager.debugGetSystemUpdateList(type);

				// Print system type
				if (updateList.size() > 0)
				{
					str += System::typeToString(type) + "\n";
				}
				else
				{
					continue;
				}

				// Print systems of this type, in the order they update
				for (auto const& system : updateList)
				{
					str += "    " + std::string(typeid(*system.get()).name()) + " -- priority: " + std::to_string(system->m_priority) + "\n";
				}
			}
			return str;
		}

	public:
		virtual ~Scene() = default;

		virtual void init() {}
		virtual void destroyed() {}

		virtual void update(const float dt)
		{
			if (m_hitStun > 0.0f)
			{
				m_hitStun = Math::approach(m_hitStun, 0.0f, Time::deltaSeconds);
				return;
			}
			
			mSystemManager.update();
		}
		
		virtual void draw()
		{
			mSystemManager.draw();
		}

		void setHitstun(float amt)
		{
			m_hitStun = amt;
		}

		float getHitstun()
		{
			return m_hitStun;
		}
	};
}

#endif // _SCENE_H