#include "../Button.h"

#include <Content.h>

#include <engine/Ecs.h>
#include <engine/Spatial.h>
#include <engine/Input.h>
#include <engine/Graphics.h>

#include "../MouseCollider.h"
#include <engine/components/Animator.h>
#include <engine/components/Transform2D.h>

using namespace Engine;

void UpdateButton::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<Button>());
    sig.set(m_scene->getComponentType<MouseCollider>());
    sig.set(m_scene->getComponentType<Animator>());
    setup(0, Type::Update, sig);
}

void UpdateButton::update()
{
    for (Entity ent : m_entities)
    {
        auto &button = m_scene->getComponent<Button>(ent);
        auto &mouseCollider = m_scene->getComponent<MouseCollider>(ent);
        auto &animator = m_scene->getComponent<Animator>(ent);

        if (button.disabled)
        {
            continue;
        }

        if (m_game->m_input.mLeftPressed())
        {
            if (button.pressedCallback && mouseCollider.rect.contains(m_game->m_input.mousePos()))
            {
                button.pressedCallback(ent);
            }
        }

        if (button.showPressedAnim)
        {
            if (m_game->m_input.mLeft() && mouseCollider.rect.contains(m_game->m_input.mousePos()))
            {
                // animator.sprite->SetAnimationInstant(Content::aButtonDown);
                animator.animation = Content::aButtonDown;
            }
            else
            {
                // animator.sprite->SetAnimationInstant(Content::aButtonUp);
                animator.animation = Content::aButtonUp;
            }
        }
    }
}
