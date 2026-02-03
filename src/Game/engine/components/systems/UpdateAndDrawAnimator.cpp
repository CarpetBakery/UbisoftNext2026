#include <engine/components/Animator.h>

#include <Content.h>

#include <engine/Ecs.h>
#include <engine/Math.h>
#include <engine/Time.h>

#include <engine/components/Transform2D.h>

using namespace Engine;

namespace
{
    class SortlistEntry
    {
    public:
        Transform2D *m_transform = nullptr;
        Animator *m_animator = nullptr;

        SortlistEntry(Transform2D *transform, Animator *animator)
            : m_transform(transform), m_animator(animator)
        {}
    };
}

void UpdateAndDrawAnimator::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<Transform2D>());
    sig.set(m_scene->getComponentType<Animator>());
    setup(0, Type::Draw, sig);
}

void UpdateAndDrawAnimator::update()
{
    // For depth sorting, we just add all animators to a list and then sort them
    std::vector<SortlistEntry> sortedEntites;

    for (Entity ent : m_entities)
    {
        auto &transform = m_scene->getComponent<Transform2D>(ent);
        auto &animator = m_scene->getComponent<Animator>(ent);

        sortedEntites.emplace_back(&transform, &animator);
    }

    // Sort by depth
    std::sort(sortedEntites.begin(), sortedEntites.end(), 
        [](const SortlistEntry &ls, const SortlistEntry &rs)
        {
            return ls.m_animator->depth < rs.m_animator->depth;
        });

    for (SortlistEntry &entry : sortedEntites)
    {
        auto *transform = entry.m_transform;
        auto *animator = entry.m_animator;

        if (animator->lerpOffset)
        {
            animator->offset.x = Math::damp(animator->offset.x, 0.0f, animator->lerpOffsetFac, Time::deltaSeconds * 10.0f);
            animator->offset.y = Math::damp(animator->offset.y, 0.0f, animator->lerpOffsetFac, Time::deltaSeconds * 10.0f);
        }

        if (animator->origin == Sprite::Origin::Custom)
        {
            animator->sprite->SetOrigin(animator->originCustom);
        }
        else
        {
            animator->sprite->SetOrigin(animator->origin);
        }

        animator->sprite->SetAngle(transform->rotation);
        
        // Draw the actual sprite
        animator->sprite->DrawExCam(
            transform->pos + animator->offset - Vec2i(0, transform->z),
            transform->scale,
            animator->animation,
            animator->color);
    }
}