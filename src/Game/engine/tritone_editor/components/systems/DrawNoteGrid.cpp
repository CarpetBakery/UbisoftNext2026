#include "../NoteGrid.h"

#include <engine/Graphics.h>
#include <engine/Input.h>

#include "../../TritoneEditorScene.h"
#include "../TritoneEditor.h"

using namespace Engine;

void DrawNoteGrid::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<NoteGrid>());
    setup(0, Type::Draw, sig);
}

void DrawNoteGrid::update()
{
    for (auto const &ent : m_entities)
    {
        auto &grid = m_scene->getComponent<NoteGrid>(ent);
        // drawNoteTest(grid);
    }
}

void DrawNoteGrid::drawNoteTest(const NoteGrid &grid)
{
    {
        // NoteGrid::Note note;
        // note.pos = Vec2i(10, 10);

        // // Make sure length can't be zero or negative
        // note.length = Math::Max(getGridMousePos(grid).x - note.pos.x, 1);

        // drawNote(grid, note);
    }

    // Test rects
    auto mPos = m_game->m_input.mousePos();
    Recti r = Recti(200, 200, 200, 200);
    Recti r2 = Recti(mPos, Vec2i(50, 100));
    Graphics::drawRect(r2, Color::green);

    if (r.overlaps(r2))
    {
        Graphics::drawRect(r, Color::red);
    }
    else
    {
        Graphics::drawRect(r, Color::vsBlue);
    }

    // Recti r = Recti(note.pos.x, note.pos.y,
    //                 m_game->m_input.mousePos().x - note.pos.x, m_game->m_input.mousePos().y - note.pos.y);
    // Graphics::drawRect(r, Color::vsBlue);
}