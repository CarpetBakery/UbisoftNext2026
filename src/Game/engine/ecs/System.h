#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "Types.h"
#include <set>
#include <engine/Game.h>

namespace Engine
{
	class Scene;

	class System
	{
		friend class SystemManager;

	private:
		// Only ever set by setup, and read by SystemManager::registerSystem
		// If you want to get a system's signature, use the scene functions.
		// Do not try to access any other way.
		Signature m_setupSignature;

	protected:
		// Reference to the parent scene
		class Scene *m_scene = nullptr;
		// Reference to the top level game class
		class Engine::Game *m_game = nullptr;

	public:
		// -- Update priority --
		enum class Type
		{
			// PrePhysics,
			// Physics,
			// PostPhysics,
			PreUpdate,
			Update,
			PostUpdate,

			PreDraw,
			Draw,
			PostDraw,

			Count
		} m_type = Type::Update;

		// Update priority inside this system's Type
		// (Lower values will be updated first. Negative values are valid.)
		int8_t m_priority = 0;

		// List of entities to work with
		std::set<Entity> m_entities;

		System() = default;
		virtual ~System() = default;

		// Used to initialize the system
		virtual void init() {}
		// Used to initialize entities when they are first added
		virtual void entityAdded(Entity const &ent) {}
		// Called when entities are removed from a system
		virtual void entityRemoved(Entity const &ent) {}
		
		virtual void update() {}
		virtual void destroyed() {}

		std::string toString() const
		{
			return std::string(typeid(*this).name()) + ", priority: " + std::to_string(m_priority);
		}

		static std::string typeToString(Type type)
		{
			switch (type)
			{
			case Type::PreUpdate:   return "PreUpdate";
			case Type::Update:      return "Update";
			case Type::PostUpdate:  return "PostUpdate";
			case Type::PreDraw:     return "PreDraw";
			case Type::Draw:        return "Draw";
			case Type::PostDraw:    return "PostDraw";
			}
			return "";
		}

	protected:
		// Helper for setting up priority, type and signature
		// should ONLY be called in the init function
		void setup(int priority, Type type, const Signature &sig)
		{
			m_priority = priority;
			m_type = type;
			m_setupSignature = sig;
		}
	};
}

#endif // _SYSTEM_H