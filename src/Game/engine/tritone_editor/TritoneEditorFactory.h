#ifndef _TRITONE_EDITOR_FACTORY_H
#define _TRITONE_EDITOR_FACTORY_H

#include <engine/ecs/Scene.h>
#include <engine/Spatial.h>
#include <engine/Sprite.h>

#include "components/TritoneEditor.h"
#include "components/NoteGrid.h"
#include "components/Button.h"
#include "components/LoadDialog.h"

namespace Factory
{
    // Factory
    Engine::Entity createTritone(Engine::Scene *scene);
    Engine::Entity createButton(Engine::Scene *scene, Button::Callback pressedCallback, const Engine::Recti &rect, Engine::Sprite *sprite, bool showPressedAnim = true);

    Engine::Entity createSongLoadDialog(Engine::Scene *scene, const std::filesystem::path &searchPath, LoadDialog::CloseCallback closedCallback, TritoneEditor &tritone);
    Engine::Entity createLoadSampleDialog(Engine::Scene *scene,
                                          const std::filesystem::path &searchPath,
                                          LoadDialog::CloseCallback closedCallback,
                                          LoadDialog::SelectCallback selectCallback,
                                          const TritoneEditor &tritone,
                                          const NoteGrid &noteGrid);
}

#endif // _TRITONE_EDITOR_FACTORY_H