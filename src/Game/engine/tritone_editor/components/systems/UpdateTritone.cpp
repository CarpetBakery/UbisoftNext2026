#include "../TritoneEditor.h"

#include <Content.h>

#include <engine/ecs/Scene.h>
#include <engine/Input.h>

#include <engine/components/Animator.h>
#include "../Button.h"

#include "../../TritoneEditorFactory.h"

using namespace Engine;
using namespace TritoneEditCommon;

namespace
{
    // Keys you can press to switch tracks
    App::Key trackKeys[trackCount] = {
        App::KEY_1,
        App::KEY_2,
        App::KEY_3,
        App::KEY_4,
        App::KEY_5,
        App::KEY_6,
        App::KEY_7,
        App::KEY_8,

        App::KEY_Q,
        App::KEY_W,
        App::KEY_E,
        App::KEY_R,
        App::KEY_T,
        App::KEY_Y,
        App::KEY_U,
        App::KEY_I,
    };

    // Default filename for unsaved data
    std::string defaultFilename = "untitled";

    // For save dialog input
    char charMap[37] = {
        'a',
        'b',
        'c',
        'd',
        'e',
        'f',
        'g',
        'h',
        'i',
        'j',
        'k',
        'l',
        'm',
        'n',
        'o',
        'p',
        'q',
        'r',
        's',
        't',
        'u',
        'v',
        'w',
        'x',
        'y',
        'z',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '_'};
}

void UpdateTritone::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<TritoneEditor>());
    sig.set(m_scene->getComponentType<NoteGrid>());
    setup(-1, Type::Update, sig);
}

void UpdateTritone::entityAdded(Entity const &ent)
{
    // -- Initialize tritone editor entity --
    auto &tritone = m_scene->getComponent<TritoneEditor>(ent);
    auto &noteGrid = m_scene->getComponent<NoteGrid>(ent);

    // Set entity id
    tritone.entityId = ent;

    // Allocate tracks in noteGrid
    for (int i = 0; i < trackCount; i++)
    {
        addTrack(tritone, noteGrid);
    }

    // TODO: Also call this when window size changes
    windowSizeUpdated(tritone, noteGrid);

    // Setup button callbacks
    for (Entity ent : tritone.buttonEntities)
    {
        auto &button = m_scene->getComponent<Button>(ent);
        button.pressedCallback = [&tritone, &noteGrid, this](const Entity &ent)
        {
            this->mouseButtonCallback(tritone, noteGrid, ent);
        };
    }

    // Set default bottom panel mode
    setBottomPanelMode(tritone, noteGrid, TritoneEditor::BottomPanelMode::Velocity);

    initializeEditor(tritone, noteGrid);

    // Create the start of the undo stack
    pushUndo(tritone, noteGrid);

    // Load the current song file if we came in here while a song was playing
    {
        std::filesystem::path currentSongFilepath = m_game->m_tritonePlayer.getCurrentSongFilepath();
        if (!currentSongFilepath.empty())
        {
            loadTritoneEditor(tritone, noteGrid, currentSongFilepath);
            tritone.saveFilepath = currentSongFilepath;

            if (m_game->m_tritonePlayer.isPlaying())
            {
                return;
            }
        }
    }

    // Send our state to tritone playback
    m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);
}

void UpdateTritone::mouseButtonCallback(TritoneEditor &tritone, NoteGrid &noteGrid, Entity const &ent)
{
    if (ent == tritone.velocityButton)
    {
        setBottomPanelMode(tritone, noteGrid, TritoneEditor::BottomPanelMode::Velocity);
    }
    else if (ent == tritone.panButton)
    {
        setBottomPanelMode(tritone, noteGrid, TritoneEditor::BottomPanelMode::Pan);
    }
    else if (ent == tritone.menuButton)
    {
        initLoadDialog(tritone, noteGrid);
    }
    else if (ent == tritone.trackButton)
    {
        initSampleLoadDialog(tritone, noteGrid);
    }
    else if (ent == tritone.playButton)
    {
        togglePlayback(tritone, noteGrid);
    }
}

void UpdateTritone::loadSongCallback(TritoneEditor &tritone, NoteGrid &noteGrid, bool cancelled, const std::filesystem::path &path)
{
    if (!cancelled)
    {
        // Load the file
        if (loadTritoneEditor(tritone, noteGrid, path))
        {
            printf("Failed to load %s\n", path.string().c_str());
        }
        else
        {
            // File loaded successfully
            tritone.saveFilepath = path;
            printf("Loaded %s\n", path.string().c_str());

            // Prevent bug where your first undo will return to an initialized file
            tritone.undoStack.pop();
            pushUndo(tritone, noteGrid);

            // Load the new samples
            m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);
        }
    }

    switchState(tritone, noteGrid, TritoneEditor::State::Main);
}

void UpdateTritone::sampleLoadCallback(TritoneEditor &tritone, NoteGrid &noteGrid, bool cancelled, const std::filesystem::path &path)
{
    if (!cancelled)
    {
        // sampleClickCallback(tritone, noteGrid, path);
    }

    switchState(tritone, noteGrid, TritoneEditor::State::Main);
}

void UpdateTritone::sampleClickCallback(TritoneEditor &tritone, NoteGrid &noteGrid, const std::filesystem::path &path)
{
    noteGrid.tracks.at(tritone.currentTrack).samplePath = path;

    // Send our state to tritone playback so we load the sample/apply oneshot value
    m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);

    if (!m_game->m_tritonePlayer.isPlaying())
    {
        m_game->m_tritonePlayer.requestPlayNote(47, 1, tritone.currentTrack);
    }
}

void UpdateTritone::update()
{
    for (Entity const &ent : m_entities)
    {
        auto &tritone = m_scene->getComponent<TritoneEditor>(ent);
        auto &noteGrid = m_scene->getComponent<NoteGrid>(ent);

        bool disableButtons = false;
        switch (tritone.state)
        {
        case TritoneEditor::State::Main:
            stMain(tritone, noteGrid);
            break;

        case TritoneEditor::State::Playing:
            editorNavigation(tritone, noteGrid);
            editorHandlePlaybackInput(tritone, noteGrid);
            openDialogCheck(tritone, noteGrid);
            getVisibleEvents(tritone, noteGrid);
            handlePianoClick(tritone, noteGrid);
            break;

        case TritoneEditor::State::LoadDialog:
            editorHandlePlaybackInput(tritone, noteGrid);
            break;

        case TritoneEditor::State::SaveDialog:
            stSaveDialog(tritone, noteGrid);
            break;
        }
    }
}

void UpdateTritone::editorNavigation(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    handleScrolling(tritone, noteGrid);

    // Switch tracks
    {
        bool changedTrack = false;
        for (int i = 0; i < trackCount; i++)
        {
            if (m_game->m_input.keyPressed(trackKeys[i]))
            {
                switchTrack(tritone, noteGrid, i);
                changedTrack = true;
                break;
            }
        }

        if (changedTrack)
        {
            m_game->m_tritonePlayer.requestPlayNote(47, 1, tritone.currentTrack);
            printf("Changed to track %d\n", tritone.currentTrack);
        }
    }
}

void UpdateTritone::editorHandlePlaybackInput(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Toggle playback
    if (m_game->m_input.keyPressed(App::KEY_SPACE))
    {
        togglePlayback(tritone, noteGrid);
    }
}

void UpdateTritone::openDialogCheck(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (tritone.state != TritoneEditor::State::Main &&
        tritone.state != TritoneEditor::State::Playing)
    {
        return;
    }

    if (m_game->m_input.keyPressed(App::KEY_CTRL_O))
    {
        initLoadDialog(tritone, noteGrid);
    }
    else if (m_game->m_input.keyPressed(App::KEY_CTRL_S))
    {
        if (tritone.saveFilepath.string().size() <= 0 || m_game->m_input.key(App::KEY_LSHIFT))
        {
            // Open dialog if we haven't saved yet
            initSaveDialog(tritone, noteGrid);
        }
        else
        {
            // Just write to file
            saveTritone(tritone, noteGrid, tritone.saveFilepath);
        }
    }
    else if (m_game->m_input.keyPressed(App::KEY_CTRL_E))
    {
        initSampleLoadDialog(tritone, noteGrid);
    }
}

void UpdateTritone::stMain(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    editorNavigation(tritone, noteGrid);
    editorHandlePlaybackInput(tritone, noteGrid);
    openDialogCheck(tritone, noteGrid);

    // New file
    // if (m_game->m_input.keyPressed(App::KEY_CTRL_N))
    //{
    //    initializeEditor(tritone, noteGrid);
    //}

    // Undo/redo
    if (m_game->m_input.keyPressed(App::KEY_CTRL_Z))
    {
        undo(tritone, noteGrid);
    }
    if (m_game->m_input.keyPressed(App::KEY_CTRL_Y))
    {
        redo(tritone, noteGrid);
    }

    // Copying
    if (m_game->m_input.keyPressed(App::KEY_CTRL_C))
    {
        copy(tritone, noteGrid);
    }
    else if (m_game->m_input.keyPressed(App::KEY_CTRL_X))
    {
        cut(tritone, noteGrid);
    }
    else if (m_game->m_input.keyPressed(App::KEY_CTRL_V))
    {
        paste(tritone, noteGrid, true);
    }

    // Move "end" back to beginning
    if (m_game->m_input.keyPressed(App::KEY_CTRL_D))
    {
        tritone.endSongMarker = 0;
    }

    // Transposing
    if (m_game->m_input.key(App::KEY_CTRL))
    {
        if (m_game->m_input.keyPressed(App::KEY_UP))
        {
            transpose(tritone, noteGrid, 1, m_game->m_input.key(App::KEY_LSHIFT));
        }
        else if (m_game->m_input.keyPressed(App::KEY_DOWN))
        {
            transpose(tritone, noteGrid, -1, m_game->m_input.key(App::KEY_LSHIFT));
        }
        else if (m_game->m_input.keyPressed(App::KEY_LEFT))
        {
            transposeEvent(tritone, noteGrid, -0.05);
        }
        else if (m_game->m_input.keyPressed(App::KEY_RIGHT))
        {
            transposeEvent(tritone, noteGrid, 0.05);
        }
    }

    // Solo/mute
    if (m_game->m_input.keyPressed(App::KEY_M))
    {
        toggleMute(tritone, noteGrid, tritone.currentTrack);
    }
    else if (m_game->m_input.keyPressed(App::KEY_CTRL_M))
    {
        toggleSolo(tritone, noteGrid, tritone.currentTrack);
    }

    // Change tempo
    {
        if (m_game->m_input.keyPressed(App::KEY_K))
        {
            tritone.bpm--;
            tritone.bpm = Math::clamp(tritone.bpm, 30, 999);
        }
        else if (m_game->m_input.keyPressed(App::KEY_L))
        {
            tritone.bpm++;
            tritone.bpm = Math::clamp(tritone.bpm, 30, 999);
        }

        if (m_game->m_input.keyPressed(App::KEY_CTRL_K))
        {
            tritone.bpm -= 10;
            tritone.bpm = Math::clamp(tritone.bpm, 30, 999);
        }
        else if (m_game->m_input.keyPressed(App::KEY_CTRL_L))
        {
            tritone.bpm += 10;
            tritone.bpm = Math::clamp(tritone.bpm, 30, 999);
        }
    }

    getVisibleEvents(tritone, noteGrid);

    // -- Note edit --
    if (tritone.editArea.contains(m_game->m_input.mousePos()))
    {
        if (tritone.isDraggingLength)
        {
            handleNoteDragging(tritone, noteGrid);
        }
        else if (m_game->m_input.mLeftPressed() || m_game->m_input.mRight())
        {
            handleNoteEditing(tritone, noteGrid);
        }

        // Detect dragging in edit area
        if (m_game->m_input.mLeft())
        {
            tritone.isDraggingLength = true;
        }
        else if (tritone.isDraggingLength)
        {
            // Stopped dragging
            tritone.isDraggingLength = false;

            if (tritone.isEditingLength)
            {
                applyLength(tritone, noteGrid);
            }
        }

        else if (!m_game->m_input.mRight() && tritone.isDragging)
        {
            tritone.isDragging = false;
            if (tritone.didEditDuringDrag)
            {
                tritone.didEditDuringDrag = false;
                pushUndo(tritone, noteGrid);
            }
        }
    }
    else if (tritone.topBarArea.contains(m_game->m_input.mousePos()))
    {
        // -- Top bar edit --
        if (m_game->m_input.mLeft())
        {
            // Set the playhead position
            Vec2i gridPos = getRelativeGridMousePos(tritone, noteGrid, m_game);
            setPlayheadStart(tritone, noteGrid, gridPos.x);
        }
        else if (m_game->m_input.mRightPressed())
        {
            Vec2i gridPos = getRelativeGridMousePos(tritone, noteGrid, m_game);
            if (tritone.endSongMarker == gridPos.x)
            {
                // Remove marker
                tritone.endSongMarker = 0;
            }
            else
            {
                // Place marker
                tritone.endSongMarker = gridPos.x;
            }
        }
    }
    else if (tritone.bottomPanelArea.contains(m_game->m_input.mousePos()))
    {
        // -- Event edit --
        if (m_game->m_input.mLeft() || m_game->m_input.mRight())
        {
            tritone.isDragging = true;
            handleEventEditing(tritone, noteGrid);
        }
        else
        {
            if (tritone.didEditDuringDrag)
            {
                tritone.isDragging = false;

                if (tritone.didEditDuringDrag)
                {
                    tritone.didEditDuringDrag = false;
                    pushUndo(tritone, noteGrid);
                }
            }
        }
    }

    handlePianoClick(tritone, noteGrid);
}

void UpdateTritone::handleScrolling(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Scroll with arrow keys
    if (!m_game->m_input.key(App::KEY_CTRL))
    {
        int moveSpd = 4;
        if (m_game->m_input.key(App::KEY_RIGHT))
        {
            scrollGrid(tritone, noteGrid, Vec2i(moveSpd, 0));
        }
        if (m_game->m_input.key(App::KEY_LEFT))
        {
            scrollGrid(tritone, noteGrid, Vec2i(-moveSpd, 0));
        }

        if (m_game->m_input.key(App::KEY_UP))
        {
            scrollGrid(tritone, noteGrid, Vec2i(0, -moveSpd));
        }
        if (m_game->m_input.key(App::KEY_DOWN))
        {
            scrollGrid(tritone, noteGrid, Vec2i(0, moveSpd));
        }
    }

    // Scroll with mouse wheel
    scrollGrid(tritone, noteGrid, Vec2i(0, m_game->m_input.mWheelDown() - m_game->m_input.mWheelUp()));

    // Middle click drag
    if (m_game->m_input.mMiddle())
    {
        if (!tritone.isMiddleClickPanning)
        {
            // Start middle click panning
            tritone.isMiddleClickPanning = true;
            tritone.panInitMousePos = m_game->m_input.mousePos();
            tritone.panInitScreenPos = tritone.screen.position();
        }

        Vec2i mouseOffset = tritone.panInitMousePos - m_game->m_input.mousePos();
        tritone.screen = Recti(tritone.panInitScreenPos + (mouseOffset / noteGrid.cellSize), tritone.screen.size());
        clampScroll(tritone, noteGrid);
    }
    else
    {
        if (tritone.isMiddleClickPanning)
        {
            // Stop middle click panning
            tritone.isMiddleClickPanning = false;
        }
    }
}

void UpdateTritone::handleNoteEditing(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Loop through visible notes
    NoteGrid::Note *noteUnderMouse = nullptr;
    for (auto &note : tritone.visibleNotes)
    {
        Recti noteCollider = noteRect(tritone, noteGrid, note);

        // Get the first note we find that's colliding with the mouse pointer
        if (noteCollider.contains(m_game->m_input.mousePos()))
        {
            noteUnderMouse = &note;
            break;
        }
    }

    // Place/delete notes with mouse
    if (m_game->m_input.mLeftPressed())
    {
        if (!noteUnderMouse)
        {
            // -- Insert new note --
            Vec2i gridPos = getRelativeGridMousePos(tritone, noteGrid, m_game);

            auto &noteMap = noteGrid.tracks.at(tritone.currentTrack).columnData;

            // Configure new note
            NoteGrid::Note newNote;
            newNote.pitch = gridPos.y;
            newNote.length = tritone.lastNoteLength;
            newNote.pos = gridPos;

            Recti noteCollider = noteRect(tritone, noteGrid, newNote);

            // Don't allow two notes to overlap by placing one before the other
            bool foundCollision = false;
            Recti otherCollider;
            for (auto const &note : tritone.visibleNotes)
            {
                otherCollider = noteRect(tritone, noteGrid, note);
                if (otherCollider.overlaps(noteCollider))
                {
                    foundCollision = true;
                    break;
                }
            }

            if (foundCollision)
            {
                // Make note smaller until we aren't colliding anymore
                do
                {
                    newNote.length--;
                } while (overlappingVisibleNote(tritone, noteGrid, newNote) && newNote.length > 0);
            }

            if (newNote.length > 0)
            {
                addNoteData(tritone, noteGrid, newNote, tritone.currentTrack);
                printf("Inserted note at %s\n", newNote.pos.toString().c_str());

                pushUndo(tritone, noteGrid);
            }

            // Play noise
            m_game->m_tritonePlayer.requestPlayNote(newNote.pitch, 1, tritone.currentTrack);
        }
        else
        {
            // TODO: Wait until the mouse is moved a small bit before starting a length edit
            // Started dragging on top of a note
            startLengthEdit(tritone, noteUnderMouse);
        }
    }
    else if (m_game->m_input.mRight())
    {
        tritone.isDragging = true;
        if (noteUnderMouse)
        {
            // Delete this note
            eraseNoteData(tritone, noteGrid, *noteUnderMouse, tritone.currentTrack);
            printf("Deleted note at %s\n", noteUnderMouse->pos.toString().c_str());

            tritone.didEditDuringDrag = true;
        }
    }
}

void UpdateTritone::handleNoteDragging(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (tritone.isEditingLength)
    {
        if (!tritone.noteToEdit)
        {
            return;
        }

        // Calculate new length of note based on mouse position
        Vec2i gridMousePos = getRelativeGridPos(tritone, noteGrid, m_game->m_input.mousePos() - Vec2i(noteGrid.cellSize.x / 2, 0));
        int newLength = gridMousePos.x - tritone.noteToEdit->pos.x + 1;

        if (newLength == tritone.editLengthPrevious)
        {
            tritone.noteToEdit->length = newLength;
            return;
        }

        tritone.noteToEdit->length = Math::Max(newLength, 1);

        // If note is colliding with another note,
        while (overlappingVisibleNote(tritone, noteGrid, *tritone.noteToEdit) && tritone.noteToEdit->length > 0)
        {
            tritone.noteToEdit->length--;
        }

        LB_ASSERT(tritone.noteToEdit->length > 0, "Note length cannot be less than 1.");
        tritone.editLengthPrevious = tritone.noteToEdit->length;
    }
}

void UpdateTritone::handleEventEditing(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    NoteGrid::EventMarker *eventUnderMouse = nullptr;
    for (auto &event : tritone.visibleEvents)
    {
        Recti eventCollider = eventRect(tritone, noteGrid, event);

        if (eventCollider.contains(m_game->m_input.mousePos()))
        {
            eventUnderMouse = &event;
            break;
        }
    }

    Vec2i gridPos = getRelativeGridMousePos(tritone, noteGrid, m_game);

    if (m_game->m_input.mLeft())
    {
        float value = 0.0f;
        float yTop = tritone.eventBgStartPos.y;
        float yMouse = m_game->m_input.mousePos().y;

        value = (yMouse - yTop) / static_cast<float>(eventMarkerRange);
        value = Math::clamp(1.0 - value, 0.0f, 1.0f);

        if (!eventUnderMouse)
        {
            // -- Insert new event --
            // Only insert new event if we're not holding this button
            if (!m_game->m_input.key(App::KEY_D))
            {
                addEventData(tritone, noteGrid, tritone.currentTrack, tritone.bottomPanelMode, gridPos.x, value);
                tritone.didEditDuringDrag = true;
            }
        }
        else
        {
            // Don't edit existing events, but do insert new ones
            if (!m_game->m_input.key(App::KEY_F))
            {
                // Edit event by erasing it, and immediately replacing it with a completely new, changed event
                eraseEventData(tritone, noteGrid, tritone.currentTrack, tritone.bottomPanelMode, gridPos.x);
                addEventData(tritone, noteGrid, tritone.currentTrack, tritone.bottomPanelMode, gridPos.x, value);
                tritone.didEditDuringDrag = true;
            }
        }
    }
    else if (m_game->m_input.mRight() && eventUnderMouse)
    {
        // Erase event data
        eraseEventData(tritone, noteGrid, tritone.currentTrack, tritone.bottomPanelMode, gridPos.x);
        tritone.didEditDuringDrag = true;
    }
}

void UpdateTritone::handlePianoClick(TritoneEditor& tritone, NoteGrid& noteGrid)
{
    if (tritone.pianoArea.contains(m_game->m_input.mousePos()))
    {
        if (m_game->m_input.mLeftPressed())
        {
            Vec2i gridPos = getRelativeGridMousePos(tritone, noteGrid, m_game);
            m_game->m_tritonePlayer.requestPlayNote(gridPos.y, 1, tritone.currentTrack);
        }
    }
}

void UpdateTritone::startLengthEdit(TritoneEditor &tritone, NoteGrid::Note *note)
{
    tritone.noteToEdit = note;
    tritone.isEditingLength = true;
    tritone.editLengthOriginal = note->length;
    tritone.editLengthPrevious = note->length;
}

void UpdateTritone::transpose(TritoneEditor &tritone, NoteGrid &noteGrid, int amt, bool allTracks)
{
    if (amt == 0)
    {
        return;
    }

    uint32_t copyStart = tritone.playHeadStartPos;
    uint32_t copyEnd = tritone.endSongMarker;

    if (copyEnd <= copyStart || copyEnd == 0)
    {
        // Can't copy
        return;
    }

    int originalTrack = tritone.currentTrack;

    // Get a copy of the notes in this range
    for (int i = 0; i < trackCount; i++)
    {
        if (allTracks)
        {
            switchTrack(tritone, noteGrid, i);

            // Don't transpose drum tracks
            if (noteGrid.tracks.at(i).oneshot)
            {
                continue;
            }
        }
        
        NoteGrid::TrackData transposeData;
        getNotesInRange(tritone, noteGrid, transposeData);
    
        for (auto &column : transposeData.columnData)
        {
            uint32_t position = column.first;
            auto &noteList = column.second;
    
            NoteGrid::NoteList replacementList;
    
            for (auto &note : noteList)
            {
                NoteGrid::Note replacementNote = note;
                replacementNote.pitch = Math::clamp(replacementNote.pitch + amt, 0, noteGrid.size.y - 1);
                replacementNote.pos.y = replacementNote.pitch;
                replacementList.insert(replacementNote);
            }
    
            // Replace list with our new thing
            noteList = replacementList;
        }
    
        // Erase the old notes
        eraseNotesInRange(tritone, noteGrid);
    
        // Paste the new notes in
        pasteNotesInRange(tritone, noteGrid, transposeData);

        if (!allTracks)
        {
            break;
        }
    }

    if (allTracks)
    {
        switchTrack(tritone, noteGrid, originalTrack);
    }

    pushUndo(tritone, noteGrid);

    printf("Transposed %d\n", amt);
}

void UpdateTritone::transposeEvent(TritoneEditor &tritone, NoteGrid &noteGrid, float amt)
{
    if (amt == 0)
    {
        return;
    }

    uint32_t copyStart = tritone.playHeadStartPos;
    uint32_t copyEnd = tritone.endSongMarker;

    if (copyEnd <= copyStart || copyEnd == 0)
    {
        // Can't copy
        return;
    }


    auto &transposeData = noteGrid.tracks.at(tritone.currentTrack);
    NoteGrid::EventMap *eventData = getEventMap(tritone, transposeData, tritone.bottomPanelMode);
    bool atLeastOneChange = false;

    for (int i = copyStart; i < copyEnd; i++)
    {
        if (!eventData->count(i))
        {
            continue;
        }

        auto &event = eventData->at(i);
        float newValue = Math::clamp(event.value + amt, 0.0f, 1.0f);

        if (newValue == event.value)
        {
            continue;
        }

        eraseEventData(tritone, noteGrid, tritone.currentTrack, tritone.bottomPanelMode, i);
        addEventData(tritone, noteGrid, tritone.currentTrack, tritone.bottomPanelMode, i, newValue);

        atLeastOneChange = true;
    }

    if (atLeastOneChange)
    {
        pushUndo(tritone, noteGrid);
    }

    printf("Transposed events %f\n", amt);
}

void UpdateTritone::applyLength(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (tritone.noteToEdit->length == tritone.editLengthOriginal)
    {
        printf("Didn't edit length\n");
    }
    else
    {
        // Create new note with new length
        NoteGrid::Note replacementNote = *tritone.noteToEdit;
        replacementNote.length = tritone.noteToEdit->length;

        // Reset old note to old length to prevent horrible glitch
        tritone.noteToEdit->length = tritone.editLengthOriginal;

        // Erase old note
        eraseNoteData(tritone, noteGrid, *tritone.noteToEdit, tritone.currentTrack, false);

        // Remove old note from visible notes, so we don't get a weird flicker...
        {
            auto it = std::find(tritone.visibleNotes.begin(), tritone.visibleNotes.end(), *tritone.noteToEdit);
            LB_ASSERT(it != tritone.visibleNotes.end(), "Could not find previous-length note to delete.");
            tritone.visibleNotes.erase(it);
        }

        // Insert new note
        addNoteData(tritone, noteGrid, replacementNote, tritone.currentTrack);
        // Insert into visible notes
        tritone.visibleNotes.push_back(replacementNote);

        // Use this new length for the next placed note
        tritone.lastNoteLength = replacementNote.length;

        // Debug print
        printf("Applied length %d\n", tritone.editLengthPrevious);
        tritone.didEditDuringDrag = true;

        pushUndo(tritone, noteGrid);
    }

    // Reset length editing vars
    tritone.isEditingLength = false;
    tritone.noteToEdit = nullptr;
    tritone.editLengthOriginal = 0;

    // NOTE: setting this to 1 might break things by skipping a check in handleNoteDragging(?)
    tritone.editLengthPrevious = 0;
}

void UpdateTritone::initLoadDialog(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    switchState(tritone, noteGrid, TritoneEditor::State::LoadDialog);

    // if (m_game->m_tritonePlayer.isPlaying())
    // {
    //     togglePlayback(tritone, noteGrid);
    // }

    std::filesystem::path bgmPath = m_game->m_tritonePlayer.m_bgmPath;
    LB_ASSERT(std::filesystem::is_directory(bgmPath), "data/bgm path not found.");

    Factory::createSongLoadDialog(m_scene, bgmPath, [&tritone, &noteGrid, this](bool cancelled, const std::filesystem::path &path)
                                  { this->loadSongCallback(tritone, noteGrid, cancelled, path); }, tritone);
}

void UpdateTritone::initSampleLoadDialog(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Cancel playback
    // if (m_game->m_tritonePlayer.isPlaying())
    // {
    //     togglePlayback(tritone, noteGrid);
    // }
    switchState(tritone, noteGrid, TritoneEditor::State::LoadDialog);

    std::filesystem::path bgmPath = m_game->m_tritonePlayer.m_samplePath;
    LB_ASSERT(std::filesystem::is_directory(bgmPath), "data/bgm/sample path not found.");

    auto closeCallback = [&tritone, &noteGrid, this](bool cancelled, const std::filesystem::path &path)
    {
        sampleLoadCallback(tritone, noteGrid, cancelled, path);
    };

    auto clickCallback = [&tritone, &noteGrid, this](const std::filesystem::path &path)
    {
        sampleClickCallback(tritone, noteGrid, path);
    };

    Factory::createLoadSampleDialog(m_scene,
                                    m_game->m_tritonePlayer.m_samplePath,
                                    closeCallback,
                                    clickCallback,
                                    tritone,
                                    noteGrid);
}

void UpdateTritone::initSaveDialog(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Setup
    tritone.saveFirstInput = true;

    if (tritone.saveFilepath != "")
    {
        tritone.filenameBuffer = tritone.saveFilepath.string();
    }
    else
    {
        tritone.filenameBuffer = defaultFilename;
    }

    if (m_game->m_tritonePlayer.isPlaying())
    {
        togglePlayback(tritone, noteGrid);
    }

    switchState(tritone, noteGrid, TritoneEditor::State::SaveDialog);
}

void UpdateTritone::toggleSolo(TritoneEditor &tritone, NoteGrid &noteGrid, int track)
{
    if (!tritone.soloMode)
    {
        // Turn on solo mode
        tritone.soloMode = true;
        tritone.soloTrack = track;

        // Mute every track
        for (int i = 0; i < trackCount; i++)
        {
            auto &trackData = noteGrid.tracks.at(i);
            trackData.muted = (i != track);
        }
    }
    else
    {
        // Turn off solo mode
        tritone.soloMode = false;
        tritone.soloTrack = -1;

        // Unmute every track
        for (auto &trackData : noteGrid.tracks)
        {
            trackData.muted = false;
        }
    }

    // Send muted state to playback
    m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);
}

void UpdateTritone::toggleMute(TritoneEditor &tritone, NoteGrid &noteGrid, int track)
{
    auto &trackToToggle = noteGrid.tracks.at(track);

    trackToToggle.muted = !trackToToggle.muted;
    if (tritone.soloMode)
    {
        // We've muted the solo track/unmuted another track, so disable solo mode
        tritone.soloMode = false;
        tritone.soloTrack = -1;
    }
    else
    {
        // See if there's only one unmuted track. If so, we're in solo mode
        int unmutedTracks = 0;
        int singleUnmutedTrack = -1;
        for (int i = 0; i < trackCount; i++)
        {
            auto &trackData = noteGrid.tracks.at(i);

            if (!trackData.muted)
            {
                unmutedTracks++;
                singleUnmutedTrack = i;

                if (unmutedTracks > 1)
                {
                    break;
                }
            }
        }

        if (unmutedTracks == 1)
        {
            // We're in solo mode again
            toggleSolo(tritone, noteGrid, singleUnmutedTrack);
        }
    }

    // Send track state to playback
    m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);
}

void UpdateTritone::stSaveDialog(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Cancel dialog
    if (m_game->m_input.keyPressed(App::KEY_ESC))
    {
        switchState(tritone, noteGrid, TritoneEditor::State::Main);
        return;
    }

    // Ubisoft... why didn't you give us "backspace", huh?!
    // I hope I'm allowed to add it to the API
    for (int i = App::KEY_A; i < App::KEY_ESC; i++)
    {
        if (m_game->m_input.keyPressed(static_cast<App::Key>(i)))
        {
            if (tritone.saveFirstInput)
            {
                tritone.saveFirstInput = false;
                tritone.filenameBuffer = "";
            }

            tritone.filenameBuffer += charMap[i];
        }
    }

    if (m_game->m_input.keyPressed(App::KEY_BACKSPACE))
    {
        if (tritone.saveFirstInput)
        {
            tritone.saveFirstInput = false;
            tritone.filenameBuffer = "";
        }

        if (tritone.filenameBuffer.size() > 0)
        {
            tritone.filenameBuffer = tritone.filenameBuffer.substr(0, tritone.filenameBuffer.size() - 1);
        }
    }

    // Submit with enter
    if (m_game->m_input.keyPressed(App::KEY_ENTER))
    {
        if (tritone.filenameBuffer.size() <= 0)
        {
            return;
        }

        // Save filename with .tri appended
        tritone.saveFilepath = m_game->m_tritonePlayer.m_bgmPath / (tritone.filenameBuffer + ".tri");

        // Save file routine
        saveTritone(tritone, noteGrid, tritone.saveFilepath);

        switchState(tritone, noteGrid, TritoneEditor::State::Main);
    }
}

void UpdateTritone::switchState(TritoneEditor &tritone, NoteGrid &noteGrid, TritoneEditor::State newState)
{
    enableMenuButtons(tritone, noteGrid);

    switch (newState)
    {
    case TritoneEditor::State::LoadDialog:
        disableMenuButtons(tritone, noteGrid);
        break;

    case TritoneEditor::State::SaveDialog:
        disableMenuButtons(tritone, noteGrid);
        break;
    }

    // Set new state
    tritone.state = newState;
}

void UpdateTritone::windowSizeUpdated(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Get pixel measurements from sprites
    tritone.beatWidth = Content::sprBg.GetWidth();
    tritone.octaveHeight = Content::sprPiano.GetHeight();
    tritone.truePianoWidth = Content::sprPiano.GetWidth() - tritone.pianoShadowWidth;
    tritone.bottomPanelHeight = Content::sprBotpanelMain.GetHeight();
    tritone.eventBgStartPos = Vec2i(tritone.drawOffset.x, m_game->getScreenHeight() - tritone.bottomPanelHeight);
    tritone.topBarHeight = Content::sprTopBar.GetHeight();

    // Offset everything down so we can draw top bar
    tritone.drawOffset.y = tritone.topBarHeight;

    // Get positions for dialogs
    // tritone.loadDialogPos = (m_game->getScreenSize() / 2) - (Content::sprLoadDialog.GetSize() / 2);
    tritone.saveDialogPos = (m_game->getScreenSize() / 2) - (Content::sprSaveDialog.GetSize() / 2);

    // Get visible area
    Vec2i screenSize = Vec2i(
                           m_game->getScreenWidth(),
                           m_game->getScreenHeight() - Content::sprBotpanelMain.GetHeight()) -
                       tritone.drawOffset;

    Vec2i visibleCells = screenSize / noteGrid.cellSize;

    // Update visibility vars
    // NOTE: we need to draw 2 more on each side so things don't get cut off while scrolling
    tritone.visibleBeats = (screenSize.x / tritone.beatWidth) + 2;
    tritone.visibleOctaves = (screenSize.y / tritone.octaveHeight) + 2;

    // Resize our view into the grid
    tritone.screen.w = visibleCells.x;
    tritone.screen.h = visibleCells.y;

    // Setup areas
    tritone.editArea = Recti(
        tritone.drawOffset + Vec2i(tritone.truePianoWidth, 0),
        tritone.screen.size() * noteGrid.cellSize);

    tritone.pianoArea = Recti(
        Vec2i(0, tritone.drawOffset.y),
        Vec2i(tritone.truePianoWidth, tritone.screen.h * noteGrid.cellSize.y));

    // I just assume that this is always at the top of the screen
    tritone.topBarArea = Recti(
        Vec2i(tritone.truePianoWidth, 0),
        Vec2i(tritone.screen.size().x * noteGrid.cellSize.x, tritone.drawOffset.y));

    tritone.bottomPanelArea = Recti(
        Vec2i(tritone.drawOffset.x + tritone.truePianoWidth, m_game->getScreenHeight() - tritone.bottomPanelHeight),
        Vec2i(m_game->getScreenWidth(), tritone.bottomPanelHeight));

    // Update button positions/hitboxes
    {
        Vec2i bottomLeft = Vec2i(tritone.drawOffset.x, m_game->getScreenHeight());

        setButtonBounds(m_scene,
                        tritone.velocityButton,
                        bottomLeft - velocityButtonOffset,
                        Content::sprVelocityButton.GetSize());

        setButtonBounds(m_scene,
                        tritone.panButton,
                        bottomLeft - panButtonOffset,
                        Content::sprPanButton.GetSize());

        setButtonBounds(m_scene,
                        tritone.menuButton,
                        menuButtonOffset,
                        Content::sprMenuButton.GetSize());

        setButtonBounds(m_scene,
                        tritone.trackButton,
                        trackButtonOffset,
                        Content::sprTrackButton.GetSize());

        setButtonBounds(m_scene,
                        tritone.playButton,
                        playButtonOffset,
                        Content::sprPlayButton.GetSize());
    }
}

void UpdateTritone::getVisibleEvents(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // -- Get visible notes --
    tritone.visibleNotes.clear();

    int startMeasure = tritone.screen.x / tritone.columnsPerMeasure;
    int endMeasure = (tritone.screen.x + (tritone.screen.w - 1)) / tritone.columnsPerMeasure;

    {
        // Loop through visible measures and add their notes to visible list
        auto &measureMap = noteGrid.tracks.at(tritone.currentTrack).measureData;

        // Used to keep track of which notes we've already added to visibleNotes
        std::unordered_set<NoteGrid::Note, NoteGrid::Note::Hash> visibleSet{};

        for (int i = startMeasure; i <= endMeasure; i++)
        {
            if (!measureMap.count(i))
            {
                // Measure has no data
                continue;
            }

            auto &measureList = measureMap.at(i);
            for (const NoteGrid::Note &note : measureList)
            {
                if (visibleSet.count(note))
                {
                    continue;
                }
                visibleSet.insert(note);
                tritone.visibleNotes.push_back(note);
            }
        }
    }

    // -- Get visible notes on other tracks --
    tritone.visibleGhostNotes.clear();
    bool isCurrentTrackDrum = noteGrid.tracks.at(tritone.currentTrack).oneshot;
    for (int i = 0; i < trackCount; i++)
    {
        if (i == tritone.currentTrack)
        {
            continue;
        }

        auto &trackData = noteGrid.tracks.at(i);
        if (trackData.muted || trackData.oneshot != isCurrentTrackDrum)
        {
            continue;
        }

        auto &measureMap = trackData.measureData;
        for (int j = startMeasure; j <= endMeasure; j++)
        {
            if (!measureMap.count(j))
            {
                continue;
            }

            auto &measureList = measureMap.at(j);
            for (const NoteGrid::Note &note : measureList)
            {
                tritone.visibleGhostNotes.insert(&note);
            }
        }
    }

    // -- Get visible events --

    tritone.visibleEvents.clear();

    // Loop through visible beats and add their events to visible list
    NoteGrid::EventMap *eventMap;
    switch (tritone.bottomPanelMode)
    {
    case TritoneEditor::BottomPanelMode::Velocity:
        eventMap = &noteGrid.tracks.at(tritone.currentTrack).velocityEvents;
        break;

    case TritoneEditor::BottomPanelMode::Pan:
        eventMap = &noteGrid.tracks.at(tritone.currentTrack).panEvents;
        break;

    default:
        break;
    }

    int startBeat = tritone.screen.x;
    int endBeat = startBeat + tritone.screen.w - 1;

    for (int i = startBeat; i < endBeat; i++)
    {
        if (!eventMap->count(i))
        {
            // Column has no data
            continue;
        }
        tritone.visibleEvents.push_back(eventMap->at(i));
    }
}

void UpdateTritone::scrollGrid(TritoneEditor &tritone, class NoteGrid &noteGrid, const Vec2i &amt)
{
    tritone.screen.x += amt.x;
    tritone.screen.y += amt.y;
    clampScroll(tritone, noteGrid);
}

void UpdateTritone::clampScroll(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Clamp inside visible area
    tritone.screen.x = Math::clamp(tritone.screen.x, 0, noteGrid.size.x - tritone.screen.w);
    tritone.screen.y = Math::clamp(tritone.screen.y, 0, noteGrid.size.y - tritone.screen.h);
}

void UpdateTritone::setBottomPanelMode(TritoneEditor &tritone, NoteGrid &noteGrid, TritoneEditor::BottomPanelMode newMode)
{
    // Switch all buttons back to being unpressed
    for (Entity ent : tritone.buttonEntities)
    {
        auto &animator = m_scene->getComponent<Animator>(ent);
        animator.animation = Content::aButtonUp;
    }

    // Get the button that was pressed
    Entity pressedButton;
    switch (newMode)
    {
    case TritoneEditor::BottomPanelMode::Velocity:
        pressedButton = tritone.velocityButton;
        break;
    case TritoneEditor::BottomPanelMode::Pan:
        pressedButton = tritone.panButton;
        break;
    }

    // Set pressed button animation
    {
        auto &animator = m_scene->getComponent<Animator>(pressedButton);
        animator.animation = Content::aButtonDown;
    }

    tritone.bottomPanelMode = newMode;
}

void UpdateTritone::setPlayheadStart(TritoneEditor &tritone, NoteGrid &noteGrid, uint32_t newStartPos)
{
    tritone.playHeadStartPos = newStartPos;
    m_game->m_tritonePlayer.requestSetPlayheadStart(newStartPos);
}

void UpdateTritone::togglePlayback(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (!m_game->m_tritonePlayer.isPlaying())
    {
        m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);

        m_game->m_tritonePlayer.play();

        switchState(tritone, noteGrid, TritoneEditor::State::Playing);
    }
    else
    {
        m_game->m_tritonePlayer.pause();
        switchState(tritone, noteGrid, TritoneEditor::State::Main);
    }
}

void UpdateTritone::disableMenuButtons(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    Entity menuButtons[3] = {
        tritone.menuButton,
        tritone.trackButton,
        tritone.playButton};

    for (int i = 0; i < 3; i++)
    {
        auto &button = m_scene->getComponent<Button>(menuButtons[i]);
        button.disabled = true;
    }
}

void UpdateTritone::enableMenuButtons(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    Entity menuButtons[3] = {
        tritone.menuButton,
        tritone.trackButton,
        tritone.playButton};

    for (int i = 0; i < 3; i++)
    {
        auto &button = m_scene->getComponent<Button>(menuButtons[i]);
        button.disabled = false;
    }
}

void UpdateTritone::switchTrack(TritoneEditor &tritone, class NoteGrid &noteGrid, int track)
{
    tritone.currentTrack = Math::clamp(track, 0, trackCount);
}

void UpdateTritone::pushUndo(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // If I had more time I would track the changes between states instead of making a copy each time
    tritone.undoStack.push(noteGrid.tracks);
    while (!tritone.redoStack.empty())
    {
        tritone.redoStack.pop();
    }
}

void UpdateTritone::undo(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (tritone.undoStack.size() <= 1)
    {
        return;
    }

    tritone.redoStack.push(tritone.undoStack.top());
    tritone.undoStack.pop();
    noteGrid.tracks = tritone.undoStack.top();
}

void UpdateTritone::redo(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (tritone.redoStack.empty())
    {
        return;
    }

    tritone.undoStack.push(tritone.redoStack.top());
    noteGrid.tracks = tritone.redoStack.top();
    tritone.redoStack.pop();
}

bool UpdateTritone::eraseNotesInRange(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    uint32_t copyStart = tritone.playHeadStartPos;
    uint32_t copyEnd = tritone.endSongMarker;

    bool madeAChange = false;

    // Remove data within range
    auto &srcData = noteGrid.tracks.at(tritone.currentTrack);
    for (int i = copyStart; i < copyEnd; i++)
    {
        if (srcData.columnData.count(i))
        {
            auto noteListCopy = srcData.columnData.at(i);
            for (auto &note : noteListCopy)
            {
                eraseNoteData(tritone, noteGrid, note, tritone.currentTrack, true);
            }

            if (noteListCopy.size() > 0)
            {
                madeAChange = true;
                // Can skip event checks this frame since eraseNoteData handles it5
                continue;
            }
        }

        // Cut velocity
        if (srcData.velocityEvents.count(i))
        {
            madeAChange = true;
            eraseEventData(tritone, noteGrid, tritone.currentTrack, TritoneEditor::BottomPanelMode::Velocity, i);
        }

        // Cut pan
        if (srcData.panEvents.count(i))
        {
            madeAChange = true;
            eraseEventData(tritone, noteGrid, tritone.currentTrack, TritoneEditor::BottomPanelMode::Pan, i);
        }
    }

    return madeAChange;
}

bool UpdateTritone::getNotesInRange(TritoneEditor &tritone, NoteGrid &noteGrid, NoteGrid::TrackData &trackData)
{
    uint32_t copyStart = tritone.playHeadStartPos;
    uint32_t copyEnd = tritone.endSongMarker;

    if (copyEnd <= copyStart || copyEnd == 0)
    {
        // Can't copy
        return true;
    }

    // Clear copyData
    trackData = NoteGrid::TrackData();

    // Copy the current track's events within a range
    // NOTE: Could probably do this with iterators/range
    auto &srcData = noteGrid.tracks.at(tritone.currentTrack);
    for (int i = copyStart; i < copyEnd; i++)
    {
        if (srcData.columnData.count(i))
        {
            trackData.columnData.insert({i, srcData.columnData.at(i)});
        }

        // Copy velocity
        if (srcData.velocityEvents.count(i))
        {
            trackData.velocityEvents.insert({i, srcData.velocityEvents.at(i)});
        }

        // Copy pan
        if (srcData.panEvents.count(i))
        {
            trackData.panEvents.insert({i, srcData.panEvents.at(i)});
        }
    }

    return false;
}

bool UpdateTritone::pasteNotesInRange(TritoneEditor &tritone, NoteGrid &noteGrid, NoteGrid::TrackData &trackData, uint32_t posDiff)
{
    auto &targetData = noteGrid.tracks.at(tritone.currentTrack);

    // -- Copy notes --
    for (auto &column : trackData.columnData)
    {
        uint32_t position = column.first + posDiff;
        for (auto &note : column.second)
        {
            NoteGrid::Note newNote;
            newNote = note;
            newNote.pos.x = position;

            addNoteData(tritone, noteGrid, newNote, tritone.currentTrack);
        }
    }

    // -- Copy velocity --
    for (auto &entry : trackData.velocityEvents)
    {
        uint32_t position = entry.first + posDiff;
        eraseEventData(tritone, noteGrid, tritone.currentTrack, TritoneEditor::BottomPanelMode::Velocity, position);
        addEventData(tritone, noteGrid, tritone.currentTrack, TritoneEditor::BottomPanelMode::Velocity, position, entry.second.value);
    }

    // -- Copy Pan --
    for (auto &entry : trackData.panEvents)
    {
        uint32_t position = entry.first + posDiff;
        eraseEventData(tritone, noteGrid, tritone.currentTrack, TritoneEditor::BottomPanelMode::Pan, position);
        addEventData(tritone, noteGrid, tritone.currentTrack, TritoneEditor::BottomPanelMode::Pan, position, entry.second.value);
    }

    return false;
}

void UpdateTritone::cut(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    if (copy(tritone, noteGrid))
    {
        return;
    }

    if (eraseNotesInRange(tritone, noteGrid))
    {
        pushUndo(tritone, noteGrid);
    }
}

bool UpdateTritone::copy(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    uint32_t copyStart = tritone.playHeadStartPos;
    uint32_t copyEnd = tritone.endSongMarker;

    if (copyEnd <= copyStart || copyEnd == 0)
    {
        // Can't copy
        return true;
    }

    tritone.initialCopyPosition = copyStart;
    tritone.copySize = tritone.endSongMarker - tritone.initialCopyPosition;

    getNotesInRange(tritone, noteGrid, tritone.copyData);
    printf("Copied %d columns of data.\n", copyEnd - copyStart);

    return false;
}

void UpdateTritone::paste(TritoneEditor &tritone, NoteGrid &noteGrid, bool movePlayhead)
{
    uint32_t posDiff = tritone.playHeadStartPos - tritone.initialCopyPosition;
    pasteNotesInRange(tritone, noteGrid, tritone.copyData, posDiff);

    if (movePlayhead)
    {
        tritone.playHeadStartPos += tritone.copySize;
    }

    printf("Pasted data at %d\n", tritone.playHeadStartPos);
    pushUndo(tritone, noteGrid);
}
