#ifndef _TRITONE_EDITOR_H
#define _TRITONE_EDITOR_H

#include <engine/ecs/System.h>
#include <engine/Spatial.h>

#include <stack>
#include <unordered_map>

#include "NoteGrid.h"

// Editor for my own music format

struct TritoneEditor
{
    // Our entity id
    Engine::Entity entityId;

    // Editor states
    enum class State
    {
        Main,
        Playing,
        LoadDialog,
        SaveDialog,
    } state = State::Main;

    // The area where you edit notes
    Engine::Recti editArea = Engine::Recti(0, 0, 0, 0);

    // The area where you click to set the playhead
    Engine::Recti topBarArea = Engine::Recti(0, 0, 0, 0);
    
    // The bottom panel area
    Engine::Recti bottomPanelArea = Engine::Recti(0, 0, 0, 0);

    // The area where the piano is drawn (used to click on keys)
    Engine::Recti pianoArea = Engine::Recti(0, 0, 0, 0);

    // Map of measures to notes
    std::unordered_map<int, NoteGrid::NoteList> measuresToNotes;

    // Notes that are visible onscreen right now
    // TODO: Turn this into a vector of pointers to avoid so many copies
    // but... will then have to deal with storing const ptrs to data inside NoteGrid::NoteList
    std::vector<NoteGrid::Note> visibleNotes{};

    // Notes that are visible, but not on this track
    // This would have to change if I wanted to show the color of each note based on what track it's from
    std::unordered_set<const NoteGrid::Note*> visibleGhostNotes;

    // Events that are visible onscreen (will be different based on the mode we're in)
    std::vector<NoteGrid::EventMarker> visibleEvents{};

    // Undo/redo
    std::stack<NoteGrid::TrackList> undoStack, redoStack;

    // Are we holding middle click to pan?
    bool isMiddleClickPanning = false;
    Engine::Vec2i panInitMousePos = Engine::Vec2i(0, 0);
    Engine::Vec2i panInitScreenPos = Engine::Vec2i(0, 0);

    // Are we dragging a note's length?
    bool isDraggingLength = false;
    
    // Are we dragging?
    bool isDragging = false;
    // Did we change at least one thing during this drag?
    bool didEditDuringDrag = false;

    // Are we dragging a note's length
    bool isEditingLength = false;
    // The length of the note we're editing on the previous frame
    int editLengthPrevious = 1;
    // The original length of the note before the edit
    int editLengthOriginal = 1;
    // The note we're currently editing
    NoteGrid::Note *noteToEdit;

    // The length of the last note we placed (used to make the next note the same length)
    int lastNoteLength = 1;

    // The track we're currently editing
    int currentTrack = 0;

    // Are we soloing a track?
    bool soloMode = false;
    // The track that's been solo'd
    int soloTrack = -1;

    // -- Bottom panel --
    enum class BottomPanelMode
    {
        Velocity,
        Pan,
        ModAmp,
        ModFreq,
        Count
    } bottomPanelMode = BottomPanelMode::Velocity;

    // Buttons
    Engine::Entity velocityButton;
    Engine::Entity panButton;
    // Menu buttons
    Engine::Entity menuButton;
    Engine::Entity trackButton;
    Engine::Entity playButton;

    // For iterating over buttons
    // NOTE: We initialize this in the factor constructor because the entities
    // haven't been created yet in here
    std::array<Engine::Entity, 5> buttonEntities{};

    // Change to shift the keyboard range up or down
    // Don't touch this...
    int keyRangeOffset = 1;

    // -- Song data --
    int timeSignatureNumerator = 4;
    int timeSignatureDenominator = 4;
    int columnsPerMeasure = timeSignatureNumerator * timeSignatureDenominator;
    uint16_t bpm = 140;

    uint32_t playHeadStartPos = 0;
    uint32_t endSongMarker = 0;

    // -- Copy data --
    uint32_t initialCopyPosition = 0;
    uint32_t copySize = 0;
    NoteGrid::TrackData copyData;

    // -- Drawing --
    // For offsetting drawing, if I decide to put a menu at the top
    // TODO: drawOffset breaks the visibility of the piano
    Engine::Vec2i drawOffset = Engine::Vec2i(0, 0);

    // Where the screen is inside of the grid (in grid cells)
    // (0, 0) is the top left corner, no scroll
    Engine::Recti screen = Engine::Recti(0, 30, 0, 0);

    // Width of one beat in pixels
    int beatWidth = 0;
    // Height of one octave in pixels
    int octaveHeight = 0;

    // How many beats should we draw, even if the last one is cut off?
    int visibleBeats = 0;
    // How many octaves should we draw, even if the last one is cut off?
    int visibleOctaves = 0;

    // -- Sprite sizes --
    // The width of the piano without the shadow...
    int truePianoWidth = 0;
    int pianoShadowWidth = 6;
    int bottomPanelHeight = 0;
    int topBarHeight = 0;

    // The position of the start of the event bg
    Engine::Vec2i eventBgStartPos = Engine::Vec2i(0, 0);
    Engine::Vec2i saveDialogPos = Engine::Vec2i(0, 0);

    // -- Saving/loading --
    bool saveFirstInput = false;
    // Name of the file we're currently editing
    std::filesystem::path saveFilepath = "";
    // Buffer that we type text to in the save dialog
    std::string filenameBuffer = "";
};

// Update the editor
class UpdateTritone : public Engine::System
{
public:
    void init() override;
    void entityAdded(Engine::Entity const &ent) override;
    void update() override;

    // States
    void switchState(TritoneEditor& tritone, NoteGrid& noteGrid, TritoneEditor::State newState);

    void stMain(TritoneEditor& tritone, NoteGrid& noteGrid);
    void stSaveDialog(TritoneEditor &tritone, NoteGrid &noteGrid);

    // Allow us to pan the view, switch tracks
    void editorNavigation(TritoneEditor &tritone, NoteGrid &noteGrid);
    // Allow us to toggle playback
    void editorHandlePlaybackInput(TritoneEditor &tritone, NoteGrid &noteGrid);
    // Check if we pressed something to open up a dialog...
    void openDialogCheck(TritoneEditor& tritone, NoteGrid& noteGrid);

    void handleScrolling(TritoneEditor &tritone, NoteGrid &noteGrid);

    void handleNoteEditing(TritoneEditor &tritone, NoteGrid &noteGrid);

    void handleNoteDragging(TritoneEditor &tritone, NoteGrid &noteGrid);
    void handleEventEditing(TritoneEditor &tritone, NoteGrid &noteGrid);

    // Allow us to click on the piano
    void handlePianoClick(TritoneEditor& tritone, NoteGrid& noteGrid);

    // -- Callbacks --
    // Called when a MouseButton button is pressed
    void mouseButtonCallback(TritoneEditor& tritone, NoteGrid& noteGrid, Engine::Entity const& ent);
    // Called when LoadDialog is closed
    void loadSongCallback(TritoneEditor& tritone, NoteGrid& noteGrid, bool cancelled, const std::filesystem::path& path);
    // Called when SampleLoadDialog is closed
    void sampleLoadCallback(TritoneEditor& tritone, NoteGrid& noteGrid, bool cancelled, const std::filesystem::path& path);
    // Called when an option is clicked in SampleLoadDialog
    void sampleClickCallback(TritoneEditor& tritone, NoteGrid& noteGrid, const std::filesystem::path& path);

    // -- Song Editing --
    // Start editing the length of a note
    void startLengthEdit(TritoneEditor &tritone, NoteGrid::Note *note);
    // Apply length to the note we were dragging
    void applyLength(TritoneEditor &tritone, NoteGrid &noteGrid);

    // Transpose a range of notes up or down
    void transpose(TritoneEditor &tritone, NoteGrid &noteGrid, int amt, bool allTracks = false);
    // Transpose a range of events up or down some value
    void transposeEvent(TritoneEditor &tritone, NoteGrid &noteGrid, float amt);

    // Switch to editing another track
    void switchTrack(TritoneEditor &tritone, NoteGrid &noteGrid, int track);
    void setBottomPanelMode(TritoneEditor &tritone, NoteGrid &noteGrid, TritoneEditor::BottomPanelMode newMode);
    void setPlayheadStart(TritoneEditor &tritone, NoteGrid &noteGrid, uint32_t newStartPos);

    void togglePlayback(TritoneEditor &tritone, NoteGrid &noteGrid);

    void disableMenuButtons(TritoneEditor& tritone, NoteGrid& noteGrid);
    void enableMenuButtons(TritoneEditor& tritone, NoteGrid& noteGrid);

    // -- Undo/redo --
    void pushUndo(TritoneEditor &tritone, NoteGrid &noteGrid);
    void undo(TritoneEditor &tritone, NoteGrid &noteGrid);
    void redo(TritoneEditor &tritone, NoteGrid &noteGrid);

    // -- Copying --
    bool eraseNotesInRange(TritoneEditor& tritone, NoteGrid& noteGrid);
    bool getNotesInRange(TritoneEditor &tritone, NoteGrid &noteGrid, NoteGrid::TrackData &trackData);
    bool pasteNotesInRange(TritoneEditor &tritone, NoteGrid &noteGrid, NoteGrid::TrackData &trackData, uint32_t posDiff = 0);

    void cut(TritoneEditor& tritone, NoteGrid& noteGrid);
    bool copy(TritoneEditor& tritone, NoteGrid& noteGrid);
    void paste(TritoneEditor& tritone, NoteGrid& noteGrid, bool movePlayhead = false);

    // -- Drawing --
    // Should call this when the window size changes
    void windowSizeUpdated(TritoneEditor &tritone, NoteGrid &noteGrid);

    // Update the list of visible notes/events
    void getVisibleEvents(TritoneEditor &tritone, NoteGrid &noteGrid);

    // Scroll the grid, but keep it clamped within the screen
    void scrollGrid(TritoneEditor &tritone, NoteGrid &noteGrid, const Engine::Vec2i &amt);
    void clampScroll(TritoneEditor &tritone, NoteGrid &noteGrid);

    // -- Save/load --
    void initLoadDialog(TritoneEditor &tritone, NoteGrid &noteGrid);
    void initSampleLoadDialog(TritoneEditor &tritone, NoteGrid &noteGrid);
    void initSaveDialog(TritoneEditor &tritone, NoteGrid &noteGrid);

    // -- Solo/mute --
    void toggleSolo(TritoneEditor& tritone, NoteGrid& noteGrid, int track);
    void toggleMute(TritoneEditor& tritone, NoteGrid& noteGrid, int track);
};

// Draw the editor elements
class DrawTritone : public Engine::System
{
public:
    void init() override;
    void update() override;

    void drawBackground(TritoneEditor& tritone, NoteGrid& noteGrid, const Engine::Vec2i &scrollPos, const Engine::Vec2i& rangeOffset);
    void drawNotes(TritoneEditor& tritone, NoteGrid& noteGrid);
    void drawPiano(TritoneEditor &tritone, NoteGrid &noteGrid);
    void drawPlayhead(TritoneEditor& tritone, NoteGrid& noteGrid);
    void drawTopMenu(TritoneEditor& tritone, NoteGrid& noteGrid);
    void drawBottomPanel(TritoneEditor& tritone, NoteGrid& noteGrid);

    void drawSaveDialog(TritoneEditor& tritone, NoteGrid& noteGrid);

    void drawDebug(TritoneEditor& tritone, NoteGrid& noteGrid);
};

namespace TritoneEditCommon
{
    // -- Song data --
    constexpr int octaveCount = 8;
    constexpr Engine::Vec2i octDrawOffset = Engine::Vec2i(7, 177);
    constexpr int trackCount = 16;
    constexpr int defaultBpm = 140;
    constexpr float velocityDefault = 0.85f;
    constexpr float panDefault = 0.5f;
    constexpr int eventMarkerRange = 122;
    
    // -- Drawing constants --

    // For centering event marker inside the cell
    constexpr Engine::Vec2i eventMarkerCellOffset = Engine::Vec2i(4, 0);
    // For centering playhead inside cell
    constexpr Engine::Vec2i playheadOffset = Engine::Vec2i(-7, 5);

    // Button offsets
    constexpr Engine::Vec2i test = Engine::Vec2i(0, 23);
    constexpr Engine::Vec2i velocityButtonOffset = Engine::Vec2i(-2, 110) + test;
    constexpr Engine::Vec2i panButtonOffset = Engine::Vec2i(-2, 78) + test;
    constexpr Engine::Vec2i menuButtonOffset = Engine::Vec2i(2, 4);
    constexpr Engine::Vec2i trackButtonOffset = Engine::Vec2i(38, 4);
    constexpr Engine::Vec2i playButtonOffset = Engine::Vec2i(106, 4);
    constexpr Engine::Vec2i endSongMarkerOffset = Engine::Vec2i(0, 16);

    constexpr Engine::Color measureTextColor = Engine::Color(210, 202, 156);
    constexpr Engine::Color buttonBgColor = Engine::Color(34, 21, 25);

    // Save dialog
    constexpr Engine::Vec2i saveTextOffset = Engine::Vec2i(17, 43);

    // -- Common functions --
    // Add a track to noteGrid
    void addTrack(TritoneEditor& tritone, class NoteGrid& noteGrid);

    // Get position inside grid, taking screen scroll into account from tritone
    Engine::Vec2i getRelativeGridPos(TritoneEditor& tritone, NoteGrid& noteGrid, const Engine::Vec2i& pos);

    // Get mouse position inside grid, taking screen scroll into account from tritone
    Engine::Vec2i getRelativeGridMousePos(TritoneEditor& tritone, NoteGrid& noteGrid, Engine::Game *game);

    // Take a cell from a grid and project it to the screen
    Engine::Vec2i gridCellToScreen(TritoneEditor& tritone, NoteGrid& noteGrid, const Engine::Vec2i& gridPos);

    // Is there a note at this position in the editor? If so, return it. If not, return nullptr
    const NoteGrid::Note* noteAtPosition(TritoneEditor& tritone, NoteGrid& noteGrid, int track, const Engine::Vec2i& gridPos);

    // Return a rectangle the size of a note
    Engine::Recti noteRect(TritoneEditor& tritone, NoteGrid& noteGrid, const NoteGrid::Note& note);

    // Return a rectangle the size of an event's column
    Engine::Recti eventRect(TritoneEditor& tritone, NoteGrid& noteGrid, const NoteGrid::EventMarker& event);

    // Get the measure that the note starts in
    int getStartMeasure(TritoneEditor& tritone, const NoteGrid::Note& note);

    // Get the measure that the note ends in
    int getEndMeasure(TritoneEditor& tritone, const NoteGrid::Note& note);

    // Are we overlapping one of the visible notes?
    bool overlappingVisibleNote(TritoneEditor& tritone, NoteGrid& noteGrid, const NoteGrid::Note& testNote, const NoteGrid::Note* overlappingNote = nullptr);

    // Insert note data and update relevant data structures
    void addNoteData(TritoneEditor& tritone, NoteGrid& noteGrid, const NoteGrid::Note& note, int track);

    // Erase note data and update relevant data structures
    void eraseNoteData(TritoneEditor& tritone, NoteGrid& noteGrid, const NoteGrid::Note& note, int track, bool eraseEventData = true);

    void addEventData(TritoneEditor& tritone,
        NoteGrid& noteGrid,
        int track,
        TritoneEditor::BottomPanelMode eventType,
        int position,
        float value = -1.0f);

    void eraseEventData(TritoneEditor& tritone,
        NoteGrid& noteGrid,
        int track,
        TritoneEditor::BottomPanelMode eventType,
        int position);

    // Convenient way to index event data by eventType
    NoteGrid::EventMap* getEventMap(TritoneEditor& tritone,
        NoteGrid::TrackData& trackData,
        TritoneEditor::BottomPanelMode eventType,
        float* defaultValueReturn = nullptr);


    void initializeEditor(TritoneEditor& tritone, NoteGrid& noteGrid);

    // Save a .tri file
    void saveTritone(TritoneEditor& tritone, NoteGrid& noteGrid, const std::filesystem::path& outPath);
    // Load a .tri file into the editor
    bool loadTritoneEditor(TritoneEditor& tritone, NoteGrid& noteGrid, const std::filesystem::path& inPath);
}


#endif // _TRITONE_EDITOR_H