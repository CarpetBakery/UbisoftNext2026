#ifndef _TRITONE_EDITOR_SCENE_H
#define _TRITONE_EDITOR_SCENE_H

#include <engine/Sprite.h>
#include <engine/ecs/Scene.h>

#include <set>

class TritoneEditorScene : public Engine::Scene
{
public:
    void init() override;
};

#endif // _TRITONE_EDITOR_SCENE_H