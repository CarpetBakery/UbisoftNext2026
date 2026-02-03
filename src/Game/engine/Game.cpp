#include "Game.h"

#include <app.h>

#include <engine/DebugConsole.h>
#include <engine/ecs/Scene.h>
#include <engine/Graphics.h>
#include <engine/Input.h>
#include <engine/Time.h>

#include <engine/tritone_editor/TritoneEditorScene.h>

using namespace Engine;

Game::Game(const GameConfig &config)
	: m_config(config)
{
}

void Game::init()
{
	// Initialize debug console
	DebugConsole::init();

	// Find the data path
	{
		namespace fs = std::filesystem;

		fs::path basePath = fs::current_path();
		fs::path currentPath;

		// Start in our current path; keep looking up until we find the data dir
		fs::path temp = basePath;
		do
		{
			LB_ASSERT(temp.string().size() > 0, "Could not find data folder.");

			currentPath = temp / "data";
			temp = temp.parent_path();
		} while (!fs::is_directory(currentPath));

		// Append a slash to the end
		currentPath /= "";
		// Set member var

		mDataPath = currentPath;
	}

	// Set window resized callback
	// glutReshapeFunc(Graphics::windowResized);

	// Initialize input
	m_input.init(this);

	// Initialize tritone performance engine
	m_tritonePlayer.init(this);
}

void Game::update(const float dt)
{
	// auto start = std::chrono::high_resolution_clock::now();

	// Update time
	Time::deltaSeconds = dt / 1000.0f;
	Time::delta = Time::deltaSeconds / Time::deltaTarget;
	Time::seconds += Time::deltaSeconds;

	// Update Input
	m_input.update();

	handleGlobalControls();

	// Update the scene
	getScene()->update(dt);

	// std::chrono::duration<float, std::milli> sleepDuration = std::chrono::high_resolution_clock::now() - start;
	// Sleep(Time::deltaTarget - sleepDuration.count());
}

void Game::draw()
{
	// Draw the scene
	getScene()->draw();
}

void Game::destroyed()
{
	// Tell the scene that it's destroyed
	getScene()->destroyed();

	m_tritonePlayer.free();

	// Free engine
	DebugConsole::free();
}

void Engine::Game::restartScene()
{
	mSceneStack.top()->restart();
	mSceneStack.top()->init();
}

void Engine::Game::handleGlobalControls()
{
	// Debug restart
	if (m_config.debugMode && m_input.keyPressed(App::KEY_0))
	{
		restartScene();
	}

	// Go fullscreen
	if (m_input.keyPressed(App::KEY_F))
	{
		glutFullScreenToggle();
	}

	// Enter tritone editor
	auto thing1 = typeid(*mSceneStack.top().get()).name();
	auto thing2 = typeid(TritoneEditorScene).name();
	if (m_input.keyPressed(App::KEY_9))
	{
		auto currentName = typeid(*mSceneStack.top().get()).name();
		auto tritoneName = typeid(TritoneEditorScene).name();
		if (currentName != tritoneName)
		{
			pushScene<TritoneEditorScene>();
		}
		else if (mSceneStack.size() > 1)
		{
			Graphics::resMult = 2.0f;
			popScene();
		}
	}
}

Scene *Game::getScene()
{
	LB_ASSERT(!mSceneStack.empty(), "Scene stack is empty.");
	return mSceneStack.top().get();
}

void Game::popScene()
{
	LB_ASSERT(!mSceneStack.empty(), "Trying to pop an empty scene stack.");

	// Tell current scene that it's destroyed
	mSceneStack.top()->destroyed();
	// Remove from the stack
	mSceneStack.pop();
}

Sprite Game::createSprite(const std::filesystem::path &relativePath, const unsigned int nColumns, const unsigned int nRows)
{
	std::filesystem::path path = getDataPath() / relativePath;
	return Sprite(this, path, nColumns, nRows);
}

std::filesystem::path Game::getDataPath() const
{
	return mDataPath;
}

Vec2i Game::getScreenSize()
{
	// TODO: Use my own variables for screen width and height
	// Will have to do this after I figure out how to dynamically set the window size... damn it
	return Vec2i(APP_VIRTUAL_WIDTH, APP_VIRTUAL_HEIGHT);
}

int Game::getScreenWidth()
{
	return APP_VIRTUAL_WIDTH;
}

int Game::getScreenHeight()
{
	return APP_VIRTUAL_HEIGHT;
}