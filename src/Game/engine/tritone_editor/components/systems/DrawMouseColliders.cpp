#include "../MouseCollider.h"

#include <engine/ecs/Scene.h>
#include <engine/Graphics.h>

using namespace Engine;

void DrawMouseColliders::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<MouseCollider>());
    setup(10, Type::Draw, sig);
}

void DrawMouseColliders::update()
{
    for (auto const &ent : m_entities)
    {
        auto &mouseCollider = m_scene->getComponent<MouseCollider>(ent);

        if (mouseCollider.rect.contains(m_game->m_input.mousePos()))
        {
            Graphics::drawRect(mouseCollider.rect, Color::red);
        }
        else
        {
            Graphics::drawRect(mouseCollider.rect, Color::blue);
        }
    }
}