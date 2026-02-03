#ifndef _NOTE_GRID_H
#define _NOTE_GRID_H

#include <engine/Sprite.h>
#include <engine/Spatial.h>
#include <engine/ecs/System.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

struct NoteGrid
{
    // Represents a note in the grid
    struct Note
    {
        // Pitch of the note
        // The current scale is 0 - lowest, 87 - highest, 39 - middle C3, 36 - A3 440Hz
        uint8_t pitch = 0;
        // Length of the note (in 16ths)
        uint16_t length = 1;

        // Position inside the grid
        Engine::Vec2i pos;

        // Hashing functor (for use with unordered_set)
        struct Hash
        {
            size_t operator()(const Note &note) const
            {
                return std::hash<Engine::Vec2i>{}(note.pos);
            }
        };
        
        // Compare note positions
        constexpr bool operator==(const Note &rhs) const
        {
            return (pos == rhs.pos);
        }
        constexpr bool operator<(const Note &rhs) const
        {
            return (pitch < rhs.pitch);
        }
    };

    struct EventMarker
    {
        // Value of the event we're holding
        float value = 0.8f;

        // Position inside the note grid
        uint32_t position;
    };

    // Size of one cell in pixels
    Engine::Vec2i cellSize = Engine::Vec2i(24, 16);

    // Bounds of the grid (in grid cells)
    Engine::Vec2i size = Engine::Vec2i(100 * 16, 88); // 88 notes on a piano

    // -- Data structure of all notes in the grid --
    // List of notes on this vertical column
    using NoteList = std::unordered_set<Note, Note::Hash>;
    // Map of horizontal position to list of notes at column
    using NoteMap = std::unordered_map<uint32_t, NoteList>;
    // Map of horizontal position to event
    using EventMap = std::unordered_map<uint32_t, EventMarker>;

    struct TrackData
    {
        // For getting notes in individual columns
        NoteMap columnData{};
        // For getting notes in an entire measure of music
        NoteMap measureData{};

        // Events
        EventMap velocityEvents{};
        EventMap panEvents{};

        // Path to sample data
        std::filesystem::path samplePath = "";
        // Should the sample play only once?
        bool oneshot = false;

        // Track muted?
        bool muted = false;
    };
    
    using TrackList = std::vector<TrackData>;

    // List of track data
    TrackList tracks{};
};

class DrawNoteGrid : public Engine::System
{
public:
    void init() override;
    void update() override;

    // Test out drawing a note
    void drawNoteTest(const NoteGrid &grid);
};

// -- Helpers --
// Get the mouse position in a grid
Engine::Vec2i getGridMousePos(const NoteGrid &grid, Engine::Game *game);

// Convert a grid coord to a screen coord
Engine::Vec2i gridToScreen(const NoteGrid &grid, const Engine::Vec2i coord);

// Convert a screen coord to a grid coord
Engine::Vec2i screenToGrid(const NoteGrid &grid, const Engine::Vec2i coord);

// Draw a note to the screen using a screen position
void drawNote(const NoteGrid &grid, Engine::Sprite &sprNote, const NoteGrid::Note &note, const Engine::Vec2i &screenPosition, bool isGhost = false);

#endif // _NOTE_GRID_H