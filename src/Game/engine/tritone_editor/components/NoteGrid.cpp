#include "NoteGrid.h"

#include <engine/Graphics.h>
#include <engine/Input.h>

using namespace Engine;

Vec2i getGridMousePos(const NoteGrid &grid, Game *game)
{
    return screenToGrid(grid, game->m_input.mousePos());
}

Vec2i gridToScreen(const NoteGrid &grid, const Vec2i coord)
{
    return coord * grid.cellSize;
}

Vec2i screenToGrid(const NoteGrid &grid, const Vec2i coord)
{
    return coord / grid.cellSize;
}

void drawNote(const NoteGrid &grid, Sprite &sprNote, const NoteGrid::Note &note, const Vec2i &screenPosition, bool isGhost)
{
    Vec2i drawPos = screenPosition + Vec2i(1, 0);
    int lengthPixels = (note.length * grid.cellSize.x) - 2;
    Color color = Color::fromFloat(1.0f, 1.0f, 1.0f);

    if (isGhost)
    {
        color *= 0.4f;
    }
    
    // Draw middle of note
    sprNote.DrawEx(drawPos, Vec2f(lengthPixels, 1.0f), 1, color);

    // Draw left side of note
    sprNote.DrawEx(drawPos, Vec2f(1, 1), 0, color);
    
    // // Draw right side of note
    sprNote.DrawEx(drawPos + Vec2i(lengthPixels, 0), Vec2f(1, 1), 0, color);
}