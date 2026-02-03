#include <app.h>
#include <main.h>

#include <Content.h>

#include <engine/Game.h>

#include <engine/tritone_editor/TritoneEditorScene.h>
#include "test/SpriteTestScene.h"
#include "test/MoverTest.h"
#include "test/SpriteTestScene.h"
#include "test/MoverTest.h"
#include "test/PlayerTestScene.h"
#include "test/ExampleScene.h"
#include "scenes/GameScene.h"
#include "scenes/TitleScene.h"

using namespace Engine;
using namespace GS;

namespace
{
	GameConfig config;
	Game game = Game(config);
}

//------------------------------------------------------------------------
// Called before first update. Do any initial setup here.
//------------------------------------------------------------------------
void Init()
{
	// Initialize game systems
	game.init();
	game.m_tritonePlayer.setMasterVolume(0.22);

	// Load game content
	Content::load(&game);

	// Start our scene

	// game.pushScene<TritoneEditorScene>();
	// game.pushScene<ExampleScene>();
	// game.pushScene<SineTestScene>();
	// game.pushScene<SpriteTestScene>();
	// game.pushScene<MoverTestScene>();

	// game.pushScene<PlayerTestScene>();
	game.pushScene<TitleScene>();
	// game.pushScene<GameScene>();

	// Make the window 2x bigger and reposition
	//glutReshapeWindow(APP_VIRTUAL_WIDTH * 2, APP_VIRTUAL_HEIGHT * 2);
 //   glutPositionWindow(APP_VIRTUAL_WIDTH / 2, APP_VIRTUAL_HEIGHT / 2);
}

//------------------------------------------------------------------------
// Update your simulation here. deltaTime is the elapsed time since the last update in ms.
// This will be called at no greater frequency than the value of APP_MAX_FRAME_RATE
//------------------------------------------------------------------------
void Update(const float deltaTime)
{
	game.update(deltaTime);
}

//------------------------------------------------------------------------
// Add your display calls here (DrawLine,Print, DrawSprite.) 
// See App.h 
//------------------------------------------------------------------------
void Render()
{
	game.draw();
}

//------------------------------------------------------------------------
// Add your shutdown code here. Called when the APP_QUIT_KEY is pressed.
// Just before the app exits.
//------------------------------------------------------------------------
void Shutdown()
{
	game.destroyed();
}