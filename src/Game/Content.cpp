#include "Content.h"

#include <engine/Game.h>


using namespace Engine;
namespace fs = std::filesystem;

// Will need to redo this file at some point

void Content::load(class Game *game)
{
	m_game = game;

	fs::path gfxPath = "gfx/";
	fs::path sndPath = game->getDataPath() / "snd/";

	// -- Sprites -------------------------------------------
	{
		// -- Game sprites --
		float animSpd = 1;
		sprPlayer = game->createSprite(gfxPath / "player.png");
		sprShadow = game->createSprite(gfxPath / "shadow.png");
		sprCoin = game->createSprite(gfxPath / "coin.png");
		sprSkull = game->createSprite(gfxPath / "skull.png");

		sprGrappleArrow = game->createSprite(gfxPath / "grapple_arrow.png");
		sprGrappleRope = game->createSprite(gfxPath / "grapple_rope.png");
		sprGrappleLaser = game->createSprite(gfxPath / "red.png");

		sprCursorInner = game->createSprite(gfxPath / "cursor_inner.png");
		sprCursorOuter = game->createSprite(gfxPath / "cursor_outer.png");

		sprCeilingHook = game->createSprite(gfxPath / "ceiling_hook.png");
		sprCeilingHookEffect = game->createSprite(gfxPath / "hook_effect.png");

		// Effects
		sprSmoke1 = game->createSprite(gfxPath / "smoke1.png");
		sprSmoke2 = game->createSprite(gfxPath / "smoke2.png");

		sprMainBg = game->createSprite(gfxPath / "main_bg.png");
		sprMainBgBehind = game->createSprite(gfxPath / "main_bg_behind.png");
		sprStartBg = game->createSprite(gfxPath / "start_bg.png");
	}

	{
		// -- TriTone sprites --
		fs::path tritonePath = gfxPath / "tritone/";

		// Not sure what to make this yet
		float animSpd = 1;

		sprNote = game->createSprite(tritonePath / "note.png", 3, 1);
		sprEventMark = game->createSprite(tritonePath / "event_marker.png");
		sprPiano = game->createSprite(tritonePath / "piano.png");
		sprOctMarker = game->createSprite(tritonePath / "octave_markers.png", 8, 1);
		sprBg = game->createSprite(tritonePath / "piano_bg.png", 2, 1);

		sprBotpanelBg = game->createSprite(tritonePath / "botpanel_bg.png");
		sprBotpanelMain = game->createSprite(tritonePath / "botpanel_main.png");
		sprBotpanelLabelPan = game->createSprite(tritonePath / "botpanel_label_pan.png");
		sprBotpanelLabelVelocity = game->createSprite(tritonePath / "botpanel_label_velocity.png");

		sprVelocityButton = game->createSprite(tritonePath / "button_velocity.png", 1, 2);
		sprPanButton = game->createSprite(tritonePath / "button_pan.png", 1, 2);

		sprScrollButtons = game->createSprite(tritonePath / "scrollbar_buttons.png", 2, 1);
		sprScrollBar = game->createSprite(tritonePath / "scrollbar.png");
		sprScrollBg = game->createSprite(tritonePath / "scrollbar_bg.png");
		sprStatusBar = game->createSprite(tritonePath / "statusbar_bg.png");

		sprMenuButton = game->createSprite(tritonePath / "button_menu.png", 1, 2);
		sprPlayButton = game->createSprite(tritonePath / "button_play.png", 1, 2);
		sprStopButton = game->createSprite(tritonePath / "button_stop.png", 1, 2);
		sprTrackButton = game->createSprite(tritonePath / "button_track.png", 1, 2);

		sprTopBar = game->createSprite(tritonePath / "top_bar.png");
		sprTopBarPanel = game->createSprite(tritonePath / "top_bar_panel.png");

		sprPlayheadBar = game->createSprite(tritonePath / "playhead_bar.png");
		sprPlayheadArrow = game->createSprite(tritonePath / "playhead.png");

		sprEndSongMarker = game->createSprite(tritonePath / "end_marker.png");

		sprLoadDialog = game->createSprite(tritonePath / "dialog_open_file.png");
		sprLoadSampleDialog = game->createSprite(tritonePath / "dialog_load_sample.png");
		sprSaveDialog = game->createSprite(tritonePath / "dialog_save_file.png");

		sprCheckboxDrum = game->createSprite(tritonePath / "checkbox_drum.png", 1, 2);

		// Animations
		sprNote.CreateAnimation(aNoteLeft, animSpd, { 0 });
		sprNote.CreateAnimation(aNoteMid, animSpd, { 1 });
		sprNote.CreateAnimation(aNoteRight, animSpd, { 2 });

		for (int i = 0; i < 8; i++)
		{
			sprOctMarker.CreateAnimation(i, animSpd, { i });
		}

		sprBg.CreateAnimation(aPianoBgLight, animSpd, { 0 });
		sprBg.CreateAnimation(aPianoBgDark, animSpd, { 1 });

		sprVelocityButton.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprVelocityButton.CreateAnimation(aButtonDown, animSpd, { 1 });

		sprPanButton.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprPanButton.CreateAnimation(aButtonDown, animSpd, { 1 });

		sprScrollButtons.CreateAnimation(aScrollButtonLeft, animSpd, { 0 });
		sprScrollButtons.CreateAnimation(aScrollButtonRight, animSpd, { 1 });

		sprScrollBar.CreateAnimation(aScrollbarLeft, animSpd, { 0 });
		sprScrollBar.CreateAnimation(aScrollbarMid, animSpd, { 1 });
		sprScrollBar.CreateAnimation(aScrollbarRight, animSpd, { 2 });

		sprMenuButton.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprMenuButton.CreateAnimation(aButtonDown, animSpd, { 1 });

		sprPlayButton.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprPlayButton.CreateAnimation(aButtonDown, animSpd, { 1 });

		sprStopButton.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprStopButton.CreateAnimation(aButtonDown, animSpd, { 1 });

		sprTrackButton.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprTrackButton.CreateAnimation(aButtonDown, animSpd, { 1 });

		sprCheckboxDrum.CreateAnimation(aButtonUp, animSpd, { 0 });
		sprCheckboxDrum.CreateAnimation(aButtonDown, animSpd, { 1 });
	}

	// -- Sounds --------------------------------------------
	sndGrappleImpact = sndPath / "grapple_impact.wav";
	sndGrappleHook = sndPath / "grapple_hook.wav";
	sndGrappleShoot = sndPath / "grapple_shoot.wav";
	sndGrappleExtend = sndPath / "grapple_extend.wav";
	sndGrappleReel = sndPath / "grapple_reel.wav";
	sndHookEffect = sndPath / "hook_effect.wav";
	sndLockOn = sndPath / "lock_on.wav";

	sndDeath = sndPath / "death.wav";
	sndDeathPlayer = sndPath / "death_player.wav";

	sndSlam = sndPath / "slam.wav";
}

void Content::unload()
{
}