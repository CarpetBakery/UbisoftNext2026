#ifndef _UID_H
#define _UID_H

#include <unordered_map>

#include <engine/ecs/System.h>

namespace GS
{
    struct UID
    {
        uint32_t id = 0;
    };

    class UpdateUID : public Engine::System
    {
    private:
        uint32_t m_id = 0;

        std::unordered_map<uint32_t, Engine::Entity> m_uidMap{};
    
    public:
        void init() override;
        void entityAdded(Engine::Entity const &ent) override;
        void entityRemoved(Engine::Entity const &ent) override;

        bool entityExists(const uint32_t uid);
        Engine::Entity getEntity(const uint32_t uid);
    };
}

#endif _UID_H