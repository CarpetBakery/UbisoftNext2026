#include "../TritoneEditor.h"

#include <Content.h>

#include <engine/ecs/Scene.h>
#include <engine/Input.h>

#include <engine/components/Animator.h>

using namespace Engine;
using namespace TritoneEditCommon;

namespace
{
    constexpr Vec2i muteTextOffset = Vec2i(140, 150);
}

void DrawTritone::init()
{
    Signature sig;
    sig.set(m_scene->getComponentType<TritoneEditor>());
    sig.set(m_scene->getComponentType<NoteGrid>());
    setup(-10, Type::Draw, sig);
}

void DrawTritone::update()
{
    for (Entity ent : m_entities)
    {
        auto &tritone = m_scene->getComponent<TritoneEditor>(ent);
        auto &noteGrid = m_scene->getComponent<NoteGrid>(ent);

        // Our scrolled position in pixels
        Vec2i scrollPos = (tritone.screen.position() * noteGrid.cellSize);
        Vec2i rangeOffset = Vec2i(0, tritone.octaveHeight - noteGrid.cellSize.y * tritone.keyRangeOffset);

        drawBackground(tritone, noteGrid, scrollPos, rangeOffset);
        drawNotes(tritone, noteGrid);
        drawPlayhead(tritone, noteGrid);
        drawPiano(tritone, noteGrid);
        drawTopMenu(tritone, noteGrid);
        drawBottomPanel(tritone, noteGrid);

        {
            // Draw current track
            Vec2i drawPos = Vec2i(15, m_game->getScreenHeight() - 28);
            Graphics::drawText(drawPos,
                               "Track: " + std::to_string(tritone.currentTrack + 1),
                               measureTextColor,
                               GLUT_BITMAP_9_BY_15);
            // Draw BPM
            drawPos.y -= 20;
            Graphics::drawText(drawPos,
                               "BPM: " + std::to_string(tritone.bpm),
                               measureTextColor,
                               GLUT_BITMAP_9_BY_15);
        }

        // Draw solo/muted
        {
            auto& currentTrack = noteGrid.tracks.at(tritone.currentTrack);
            if (tritone.soloTrack == tritone.currentTrack && tritone.soloMode)
            {
                // Draw solo text
                Graphics::drawText(m_game->getScreenSize() - muteTextOffset, "TRACK SOLO", TritoneEditCommon::measureTextColor, GLUT_BITMAP_9_BY_15);
            }
            else if (currentTrack.muted)
            {
                // Draw muted text
                Graphics::drawText(m_game->getScreenSize() - muteTextOffset, "TRACK MUTED", TritoneEditCommon::measureTextColor, GLUT_BITMAP_9_BY_15);
            }
        }

        // -- Draw dialogs --
        switch (tritone.state)
        {
        case TritoneEditor::State::SaveDialog:
            drawSaveDialog(tritone, noteGrid);
            break;
        }

        // -- Debug draw --
        if (false)
        {
            drawDebug(tritone, noteGrid);
        }
    }
}

void DrawTritone::drawBackground(TritoneEditor &tritone, NoteGrid &noteGrid, const Engine::Vec2i &scrollPos, const Engine::Vec2i &rangeOffset)
{
    // Scroll pos but with x modded to be within beatWidth
    Vec2i loopedScrollPos = Vec2i(scrollPos.x % tritone.beatWidth, scrollPos.y);

    Vec2i drawPosInit = Vec2i(tritone.truePianoWidth, 0) + tritone.drawOffset - loopedScrollPos - rangeOffset;
    Vec2i drawPos = drawPosInit;

    int beatNumber = tritone.screen.x / tritone.timeSignatureNumerator;
    int measureNumber = beatNumber / tritone.timeSignatureDenominator;

    int anim = Content::aPianoBgLight;

    // Fix stupid bug where the measure number gets messed up as soon as it goes offscreen
    if ((scrollPos.x / tritone.beatWidth) % 4 > 0)
    {
        measureNumber += 1;
    }

    for (int i = 0; i < tritone.visibleBeats; i++)
    {
        // Choose correct animation
        if (((i + beatNumber) / 4) % 2 == 0)
        {
            anim = 0;
        }
        else
        {
            anim = 1;
        }

        for (int j = 0; j < tritone.visibleOctaves; j++)
        {
            Vec2i wrapOffset = Vec2i(0, 0);
            while ((drawPos.y + tritone.octaveHeight) + wrapOffset.y < tritone.drawOffset.y)
            {
                wrapOffset.y += tritone.visibleOctaves * tritone.octaveHeight;
            }

            Content::sprBg.DrawEx(drawPos + wrapOffset, Vec2f(1, 1), anim);
            drawPos.y += tritone.octaveHeight;
        }

        // Draw top bar
        Content::sprTopBar.DrawEx(Vec2i(drawPos.x, 0));

        // Print measure numbers
        if ((i + beatNumber) % 4 == 0)
        {
            App::Print(drawPos.x + 4,
                       m_game->getScreenHeight() - 20,
                       std::to_string(measureNumber).c_str(),
                       measureTextColor.r / 255.0f,
                       measureTextColor.g / 255.0f,
                       measureTextColor.b / 255.0f,
                       GLUT_BITMAP_8_BY_13);
            measureNumber++;
        }

        drawPos.x += tritone.beatWidth;
        drawPos.y = drawPosInit.y;
    }
}

void DrawTritone::drawNotes(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Draw ghost notes
    for (const NoteGrid::Note *note : tritone.visibleGhostNotes)
    {
        // Draw each note in this column
        Vec2i _drawPos = gridCellToScreen(tritone, noteGrid, note->pos);
        _drawPos += Vec2i(0, 3); // Offset note into middle of the grid
        drawNote(noteGrid, Content::sprNote, *note, _drawPos, true);
    }

    // Draw this track's notes
    for (auto const &note : tritone.visibleNotes)
    {
        // Draw each note in this column
        Vec2i _drawPos = gridCellToScreen(tritone, noteGrid, note.pos);
        _drawPos += Vec2i(0, 3); // Offset note into middle of the grid
        drawNote(noteGrid, Content::sprNote, note, _drawPos);
    }
}

void DrawTritone::drawPlayhead(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Draw playhead
    if (m_game->m_tritonePlayer.isPlaying())
    {
        Vec2i playheadPos = gridCellToScreen(tritone, noteGrid, Vec2i(m_game->m_tritonePlayer.playheadPos(), 0));
        playheadPos.y = 0;
        Content::sprPlayheadBar.DrawEx(playheadPos, Vec2i(1, m_game->getScreenHeight()));
    }

    // Draw playhead start pos
    if (tritone.playHeadStartPos != 0)
    {
        Vec2i playheadStartPos = gridCellToScreen(tritone, noteGrid, Vec2i(tritone.playHeadStartPos, 0));
        playheadStartPos.y = 10;
        Content::sprPlayheadArrow.DrawEx(playheadStartPos + playheadOffset);
    }

    // Draw end marker pos
    if (tritone.endSongMarker != 0)
    {
        Vec2i endMarkerPos = gridCellToScreen(tritone, noteGrid, Vec2i(tritone.endSongMarker, 0));
        endMarkerPos.y = 0;
        Content::sprEndSongMarker.DrawEx(endMarkerPos + endSongMarkerOffset);
    }
}

void DrawTritone::drawTopMenu(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    Content::sprTopBarPanel.DrawEx(Vec2i(0, 0), Vec2i(1, 1));

    // Set correct play button sprite
    {
        auto &animator = m_scene->getComponent<Animator>(tritone.playButton);
        animator.sprite = m_game->m_tritonePlayer.isPlaying() ? &Content::sprStopButton : &Content::sprPlayButton;
    }
}

void DrawTritone::drawBottomPanel(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Bottom panel y not affected by draw offset
    Vec2i drawPosInit = tritone.eventBgStartPos;
    Vec2i drawPos = drawPosInit;

    // Draw bg first
    drawPos.x += Content::sprBotpanelMain.GetWidth() + Content::sprBotpanelLabelVelocity.GetWidth();
    for (int i = 0; i < tritone.visibleBeats; i++)
    {
        int xOffset = (tritone.screen.x % tritone.timeSignatureDenominator) * noteGrid.cellSize.x;

        Content::sprBotpanelBg.DrawEx(drawPos - Vec2i(xOffset, 0));
        drawPos.x += Content::sprBotpanelBg.GetWidth();
    }
    
    drawPos = drawPosInit;
    Content::sprBotpanelMain.DrawEx(drawPos);

    // Label
    {
        drawPos.x += Content::sprBotpanelMain.GetWidth();

        Sprite *sprBottomLabel;
        switch (tritone.bottomPanelMode)
        {
        case TritoneEditor::BottomPanelMode::Velocity:
            sprBottomLabel = &Content::sprBotpanelLabelVelocity;
            break;
        case TritoneEditor::BottomPanelMode::Pan:
            sprBottomLabel = &Content::sprBotpanelLabelPan;
            break;
        }

        sprBottomLabel->DrawEx(drawPos);
    }

    // Setup for drawing bottom stuff
    drawPos.x = Content::sprBotpanelMain.GetWidth();
    drawPos.y = m_game->getScreenHeight() - Content::sprStatusBar.GetHeight();

    // Status bar
    Content::sprStatusBar.DrawEx(drawPos, Vec2f(m_game->getScreenWidth() - drawPos.x, 1));

    // -- Draw events --
    for (auto &event : tritone.visibleEvents)
    {
        Vec2i _drawPos = Vec2i(0, 0);

        // Get correct x position
        _drawPos.x = gridCellToScreen(tritone, noteGrid, Vec2i(event.position, 0)).x;

        // Get correct y position
        int yOffset = eventMarkerRange - (eventMarkerRange * event.value);
        yOffset = Math::clamp(yOffset, 0, eventMarkerRange);
        _drawPos.y = drawPosInit.y + yOffset;

        // Offset nicely into cell
        _drawPos += eventMarkerCellOffset;

        Content::sprEventMark.DrawEx(_drawPos);
    }
}

void DrawTritone::drawSaveDialog(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    Content::sprSaveDialog.DrawEx(tritone.saveDialogPos);
    Graphics::drawText(tritone.saveDialogPos + saveTextOffset,
                       tritone.filenameBuffer + ".tri",
                       measureTextColor, GLUT_BITMAP_9_BY_15);
}

void DrawTritone::drawDebug(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    // Draw track number
    App::Print(200, 200, std::to_string(tritone.currentTrack).c_str());

    // Draw areas
    //if (false)
    {
        Graphics::drawRect(tritone.editArea, Color::red);
        Graphics::drawRect(tritone.pianoArea, Color::vsBlue);
        Graphics::drawRect(tritone.topBarArea, Color::green);
        Graphics::drawRect(tritone.bottomPanelArea, Color::blue);
    }

    Graphics::drawText(Vec2i(200, 20), "TEST_DATA.TRI", Color::white, GLUT_BITMAP_9_BY_15);
}

void DrawTritone::drawPiano(TritoneEditor &tritone, NoteGrid &noteGrid)
{
    auto &sprPiano = Content::sprPiano;
    auto &sprOctMarker = Content::sprOctMarker;

    // Piano only scroll vertically
    Vec2i drawPos = tritone.drawOffset;
    drawPos.y -= (tritone.screen.position().y * noteGrid.cellSize.y);                 // Add scroll offset
    drawPos.y -= tritone.octaveHeight - noteGrid.cellSize.y * tritone.keyRangeOffset; // Add range offset

    // We start drawing from the top, so the highest octave is first
    int octIndex = octaveCount - 1;

    sprPiano.SetOrigin(Sprite::Origin::TopLeft);
    for (int i = 0; i < tritone.visibleOctaves; i++)
    {
        Vec2i wrapOffset = Vec2i(0, 0);

        // If we're above the screen completely, wrap around to the bottom
        int j = 0;
        while ((drawPos.y + tritone.octaveHeight) + wrapOffset.y < tritone.drawOffset.y)
        {
            wrapOffset.y += tritone.visibleOctaves * tritone.octaveHeight;
            j += tritone.visibleOctaves;
        }
        sprPiano.DrawEx(drawPos + wrapOffset);

        // Draw octave markers
        sprOctMarker.DrawEx(drawPos + wrapOffset + octDrawOffset, Vec2i(1, 1), Math::Max(octIndex - j, 0));
        octIndex = Math::Max(octIndex - 1, 0);

        drawPos.y += tritone.octaveHeight;
    }
}