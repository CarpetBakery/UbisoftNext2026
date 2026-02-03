#include "TritoneEditor.h"

#include <engine/Input.h>
#include <engine/DebugConsole.h>
#include <Content.h>

using namespace Engine;

namespace TritoneEditCommon
{
    Vec2i getRelativeGridPos(TritoneEditor &tritone, NoteGrid &noteGrid, const Vec2i &pos)
    {
        Vec2i newPos = pos;

        // HACK: We need to offset to where the grid background
        // is actually being drawn
        newPos += Vec2i(-tritone.truePianoWidth, 0) + tritone.drawOffset;

        // Get the offsetted mouse position inside the grid
        Vec2i gridPos = screenToGrid(noteGrid, newPos);
        // Need to offset again by the screen scroll amount
        gridPos += tritone.screen.position();

        // Reverse the y-axis to mirror how notes are stored in the data structure
        // where low y-values map to lower pitches
        gridPos.y = (noteGrid.size.y - 1) - gridPos.y;

        // Clamp inside the grid size
        gridPos.x = Math::clamp(gridPos.x, 0, (noteGrid.size.x - 1));
        gridPos.y = Math::clamp(gridPos.y, 0, (noteGrid.size.y - 1));

        return gridPos;
    }

    Vec2i getRelativeGridMousePos(TritoneEditor &tritone, NoteGrid &noteGrid, Game *game)
    {
        return getRelativeGridPos(tritone, noteGrid, game->m_input.mousePos());
    }

    Vec2i gridCellToScreen(TritoneEditor &tritone, NoteGrid &noteGrid, const Vec2i &gridPos)
    {
        Vec2i screenPos = gridPos;

        // Convert pitch y-axis to raw y-axis
        screenPos.y = (noteGrid.size.y - 1) - screenPos.y;

        // Get grid pos relative to screen position
        screenPos -= tritone.screen.position();

        // Convert grid pos to pos in pixels
        screenPos *= noteGrid.cellSize;

        // Add offsets
        // screenPos += Vec2i(tritone.truePianoWidth, 0) + tritone.drawOffset;
        screenPos += Vec2i(tritone.truePianoWidth, 0) - tritone.drawOffset;

        return screenPos;
    }

    const NoteGrid::Note *noteAtPosition(TritoneEditor &tritone, NoteGrid &noteGrid, int track, const Vec2i &gridPos)
    {
        auto &noteMap = noteGrid.tracks.at(track).columnData;

        if (!noteMap.count(gridPos.x))
        {
            return nullptr;
        }

        // TODO: Do something better than this linear search
        for (auto const &note : noteMap.at(gridPos.x))
        {
            if (note.pitch == gridPos.y)
            {
                return &note;
            }
        }

        return nullptr;
    }

    Recti noteRect(TritoneEditor &tritone, NoteGrid &noteGrid, const NoteGrid::Note &note)
    {
        return Recti(
            gridCellToScreen(tritone, noteGrid, note.pos),
            Vec2i(note.length * noteGrid.cellSize.x, noteGrid.cellSize.y));
    }

    Recti eventRect(TritoneEditor &tritone, NoteGrid &noteGrid, const NoteGrid::EventMarker &event)
    {
        Vec2i pos = Vec2i(
            gridCellToScreen(tritone, noteGrid, Vec2i(event.position, 0)).x,
            tritone.eventBgStartPos.y);

        Recti rect = Recti(
            pos + Vec2i(eventMarkerCellOffset.x, 2),
            Vec2i(Content::sprEventMark.GetWidth(), eventMarkerRange));

        return rect;
    }

    int getStartMeasure(TritoneEditor &tritone, const NoteGrid::Note &note)
    {
        return note.pos.x / tritone.columnsPerMeasure;
    }

    int getEndMeasure(TritoneEditor &tritone, const NoteGrid::Note &note)
    {
        return (note.pos.x + (note.length - 1)) / tritone.columnsPerMeasure;
    }

    bool overlappingVisibleNote(TritoneEditor &tritone, NoteGrid &noteGrid, const NoteGrid::Note &testNote, const NoteGrid::Note *overlappingNote)
    {
        Recti noteRectangle = noteRect(tritone, noteGrid, testNote);
        for (const auto &note : tritone.visibleNotes)
        {
            // Don't collide with self
            if (note.pos == testNote.pos)
            {
                continue;
            }

            Recti otherCollider = noteRect(tritone, noteGrid, note);
            if (otherCollider.overlaps(noteRectangle))
            {
                if (overlappingNote)
                {
                    overlappingNote = &note;
                }
                return true;
            }
        }
        return false;
    }

    void addNoteData(TritoneEditor &tritone, NoteGrid &noteGrid, const NoteGrid::Note &note, int track)
    {
        auto &trackData = noteGrid.tracks.at(track);

        // -- Insert into column data --
        auto &columnMap = trackData.columnData;

        // Make sure that there's a column to work with
        columnMap.insert({note.pos.x, NoteGrid::NoteList{}});
        auto &noteList = columnMap.at(note.pos.x);
        noteList.insert(note);

        // -- Insert into measures structure --
        auto &measureMap = trackData.measureData;

        // Figure out what measure this note is in
        int startMeasure = getStartMeasure(tritone, note);
        int endMeasure = getEndMeasure(tritone, note);

        // Add note to each measure it spans
        for (int i = startMeasure; i <= endMeasure; i++)
        {
            // Make sure there's a NoteList to work with
            measureMap.insert({i, NoteGrid::NoteList{}});

            auto &measureList = measureMap.at(i);
            measureList.insert(note);
        }

        // Insert events with default values
        addEventData(tritone, noteGrid, track, TritoneEditor::BottomPanelMode::Velocity, note.pos.x);
        addEventData(tritone, noteGrid, track, TritoneEditor::BottomPanelMode::Pan, note.pos.x);
    }

    void eraseNoteData(TritoneEditor &tritone, NoteGrid &noteGrid, const NoteGrid::Note &note, int track, bool eraseEventData)
    {
        auto &trackData = noteGrid.tracks.at(track);

        // -- Erase from column data --
        auto &noteList = trackData.columnData.at(note.pos.x);
        LB_ASSERT(noteList.count(note), "Couldn't find note we're deleting.");
        noteList.erase(note);

        if (noteList.size() <= 0)
        {
            // Erase list from map if no more notes on this column
            trackData.columnData.erase(note.pos.x);

            if (eraseEventData)
            {
                // Only erase events if this was the last note on the column
                // -- Erase velocity event --
                auto &velocityEvents = trackData.velocityEvents;
                if (velocityEvents.count(note.pos.x))
                {
                    velocityEvents.erase(note.pos.x);
                }

                // -- Erase pan event --
                auto &panEvents = trackData.panEvents;
                if (panEvents.count(note.pos.x))
                {
                    panEvents.erase(note.pos.x);
                }
            }
        }

        // -- Erase from measure data --
        auto &measureMap = trackData.measureData;

        // TODO: Address repeated code from above
        int startMeasure = getStartMeasure(tritone, note);
        int endMeasure = getEndMeasure(tritone, note);

        // Remove note from each measure it spans
        for (int i = startMeasure; i <= endMeasure; i++)
        {
            if (!measureMap.count(i))
            {
                // If we're deleting a note that was just extended into a new measure
                // this can happen, since the old note will take on the new length
                continue;
            }

            auto &measureList = measureMap.at(i);
            measureList.erase(note);
        }
    }

    void addEventData(TritoneEditor &tritone,
                      NoteGrid &noteGrid,
                      int track,
                      TritoneEditor::BottomPanelMode eventType,
                      int position,
                      float value)
    {
        auto &trackData = noteGrid.tracks.at(track);
        float defaultValue = 0.0f;

        // Get the type of event we want to create
        NoteGrid::EventMap *eventMap = getEventMap(tritone, trackData, eventType, &defaultValue);

        // Use passed in value if there is one
        if (value >= 0)
        {
            defaultValue = Math::clamp(value, 0.0f, 1.0f);
        }

        if (eventMap->count(position))
        {
            return;
        }

        // Insert the event if one doesn't already exist there
        NoteGrid::EventMarker event;
        event.value = defaultValue;
        event.position = position;
        eventMap->insert({position, event});
    }

    void eraseEventData(TritoneEditor &tritone,
                        NoteGrid &noteGrid,
                        int track,
                        TritoneEditor::BottomPanelMode eventType,
                        int position)
    {
        auto &trackData = noteGrid.tracks.at(track);
        NoteGrid::EventMap *eventMap = getEventMap(tritone, trackData, eventType);

        if (eventMap->count(position))
        {
            eventMap->erase(position);
        }
    }

    NoteGrid::EventMap *getEventMap(TritoneEditor &tritone,
                                    NoteGrid::TrackData &trackData,
                                    TritoneEditor::BottomPanelMode eventType,
                                    float *defaultValueReturn)
    {
        switch (eventType)
        {
        case TritoneEditor::BottomPanelMode::Velocity:
            if (defaultValueReturn)
            {
                *defaultValueReturn = velocityDefault;
            }
            return &trackData.velocityEvents;

        case TritoneEditor::BottomPanelMode::Pan:
            if (defaultValueReturn)
            {
                *defaultValueReturn = panDefault;
            }
            return &trackData.panEvents;
        }

        LB_ASSERT(false, "Invalid bottom panel mode.");
        return nullptr;
    }

    void initializeEditor(TritoneEditor &tritone, NoteGrid &noteGrid)
    {
        for (int i = 0; i < noteGrid.tracks.size(); i++)
        {
            auto &trackData = noteGrid.tracks.at(i);
            trackData = NoteGrid::TrackData();

            // Set the later half of the tracks to drum mode
            trackData.oneshot = i > 7;
        }

        tritone.bpm = defaultBpm;

        while (tritone.undoStack.size() > 1)
        {
            tritone.undoStack.pop();
        }
        while (!tritone.redoStack.empty())
        {
            tritone.redoStack.pop();
        }
    }

    void addTrack(TritoneEditor& tritone, class NoteGrid& noteGrid)
    {
        noteGrid.tracks.push_back(NoteGrid::TrackData{});
    }
}
