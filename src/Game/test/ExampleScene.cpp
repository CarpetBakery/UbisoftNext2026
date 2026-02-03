///////////////////////////////////////////////////////////////////////////////
// Provides a demo of how to use the API
///////////////////////////////////////////////////////////////////////////////
#include "ExampleScene.h"

#include <iostream>
#include <filesystem>

#include <app.h>
#include <engine/Spatial.h>
#include <engine/Math.h>
#include <engine/Time.h>

using namespace Engine;
using namespace GS;

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Example data....
//------------------------------------------------------------------------
static CSimpleSprite* testSprite;
enum
{
	ANIM_FORWARDS,
	ANIM_BACKWARDS,
	ANIM_LEFT,
	ANIM_RIGHT,
};

static std::filesystem::path audioPath = "TestData/Test.wav";
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// Called before first update. Do any initial setup here.
//------------------------------------------------------------------------
void ExampleScene::init()
{
	namespace fs = std::filesystem;

	fs::path basePath = fs::current_path();
	fs::path dataPath;

	// Start in our current path; keep looking up until we find the data dir
	fs::path temp = basePath;
	do
	{	
		if (temp.string().size() <= 0)
		{
			// We didn't find the data folder
			// TODO: Add an assert
			fprintf(stderr, "Error: Couldn't find the data path.");
		}

		dataPath = temp / "data";
		temp = temp.parent_path();
	} while (!fs::exists(dataPath) && !fs::is_directory(dataPath));

	dataPath /= "";
	std::cout << "basePath: " << basePath << "\ndataPath: " << dataPath << "\n";

	fs::path bmpPath = dataPath / "TestData/Test.bmp";
	audioPath = dataPath / audioPath;

	
	//------------------------------------------------------------------------
	// Example Sprite Code....
	// testSprite = App::CreateSprite("./data/TestData/Test.bmp", 8, 4);
	testSprite = App::CreateSprite(bmpPath.string().c_str(), 8, 4);
	testSprite->SetPosition(400.0f, 400.0f);
	const float speed = 1.0f / 15.0f;
	testSprite->CreateAnimation(ANIM_BACKWARDS, speed, { 0,1,2,3,4,5,6,7 });
	testSprite->CreateAnimation(ANIM_LEFT, speed, { 8,9,10,11,12,13,14,15 });
	testSprite->CreateAnimation(ANIM_RIGHT, speed, { 16,17,18,19,20,21,22,23 });
	testSprite->CreateAnimation(ANIM_FORWARDS, speed, { 24,25,26,27,28,29,30,31 });
	testSprite->SetScale(1.0f);
	//------------------------------------------------------------------------

	// Test math
	printf("Max: %d\n", Math::Max(1, 4));
	printf("Pi: %f\n", Math::pi);
}

//------------------------------------------------------------------------
// Update your simulation here. deltaTime is the elapsed time since the last update in ms.
// This will be called at no greater frequency than the value of APP_MAX_FRAME_RATE
//------------------------------------------------------------------------
void ExampleScene::update(const float dt)
{
	/*
	* Controller keyboard mapping:
	* W A S D 		- Left thumb stick
	* Arrow keys 	- Right thumb stick
	* I J K L 		- D-pad
	* T Y G H 		- A, B, X, Y face buttons respectively
	*/
	
	//------------------------------------------------------------------------
	// Example Sprite Code....
	// testSprite->Update(deltaTime);
	testSprite->Update(Time::deltaSeconds * 1000);
	if (App::GetController().GetLeftThumbStickX() > 0.5f)
	{
		testSprite->SetAnimation(ANIM_RIGHT);
		float x, y;
		testSprite->GetPosition(x, y);
		x += 1.0f;
		testSprite->SetPosition(x, y);
	}
	if (App::GetController().GetLeftThumbStickX() < -0.5f)
	{
		testSprite->SetAnimation(ANIM_LEFT);
		float x, y;
		testSprite->GetPosition(x, y);
		x -= 1.0f;
		testSprite->SetPosition(x, y);
	}
	if (App::GetController().GetLeftThumbStickY() > 0.5f)
	{
		testSprite->SetAnimation(ANIM_FORWARDS);
		float x, y;
		testSprite->GetPosition(x, y);
		y += 1.0f;
		testSprite->SetPosition(x, y);
	}
	if (App::GetController().GetLeftThumbStickY() < -0.5f)
	{
		testSprite->SetAnimation(ANIM_BACKWARDS);
		float x, y;
		testSprite->GetPosition(x, y);
		y -= 1.0f;
		testSprite->SetPosition(x, y);
	}
	
	if (App::GetController().GetRightThumbStickX() > 0.5f)
	{
		printf("Right stick - Right\n");
	}
	if (App::GetController().GetRightThumbStickX() < -0.5f)
	{
		printf("Right stick - Left\n");
	}
	if (App::GetController().GetRightThumbStickY() > 0.5f)
	{
		printf("Right stick - Up\n");
	}
	if (App::GetController().GetRightThumbStickY() < -0.5f)
	{
		printf("Right stick - Down\n");
	}

	if (App::GetController().CheckButton(App::BTN_DPAD_UP, false))
	{
		testSprite->SetScale(testSprite->GetScale() + 0.1f);
		printf("D-pad UP\n");
	}
	if (App::GetController().CheckButton(App::BTN_DPAD_DOWN, false))
	{
		testSprite->SetScale(testSprite->GetScale() - 0.1f);
		printf("D-pad DOWN\n");
	}
	if (App::GetController().CheckButton(App::BTN_DPAD_LEFT, false))
	{
		testSprite->SetAngle(testSprite->GetAngle() + 0.1f);
		printf("D-pad LEFT\n");
	}
	if (App::GetController().CheckButton(App::BTN_DPAD_RIGHT, false))
	{
		testSprite->SetAngle(testSprite->GetAngle() - 0.1f);
		printf("D-pad RIGHT\n");
	}
	if (App::GetController().CheckButton(App::BTN_A, true))
	{
		testSprite->SetAnimation(-1);
		printf("Face-button A\n");
	}
	//------------------------------------------------------------------------
	// Sample Sound.
	//------------------------------------------------------------------------
	if (App::GetController().CheckButton(App::BTN_B, true))
	{
		printf("Face-button B\n");
		// App::PlayAudio("./Data/TestData/Test.wav", true);
		App::PlayAudio(audioPath.string().c_str(), true);
	}
	if (App::GetController().CheckButton(App::BTN_X, true))
	{
		printf("Face-button X\n");
		// App::StopAudio("./Data/TestData/Test.wav");
		App::StopAudio(audioPath.string().c_str());
	}
	if (App::GetController().CheckButton(App::BTN_Y, true))
	{
		printf("Face-button Y\n");
		// App::StopAudio("./Data/TestData/Test.wav");
	}
}

//------------------------------------------------------------------------
// Add your display calls here (DrawLine,Print, DrawSprite.) 
// See App.h 
//------------------------------------------------------------------------
void ExampleScene::draw()
{
	//------------------------------------------------------------------------
	// Example Sprite Code....
	testSprite->Draw();
	//------------------------------------------------------------------------

    float x;
    float y;
    testSprite->GetPosition(x, y);

	//------------------------------------------------------------------------
	// Example Text.
	//------------------------------------------------------------------------
	App::Print(100, 100, "Sample Text");
	App::Print(x, y, "Sample Text");

	//------------------------------------------------------------------------
	// Example Line Drawing.
	//------------------------------------------------------------------------
	static float a = 0.0f;
	const float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	a += 0.1f;
	for (int i = 0; i < 20; i++)
	{

		const float sx = 200 + sinf(a + i * 0.1f) * 60.0f;
		const float sy = 200 + cosf(a + i * 0.1f) * 60.0f;
		const float ex = 700 - sinf(a + i * 0.1f) * 60.0f;
		const float ey = 700 - cosf(a + i * 0.1f) * 60.0f;
		g = (float)i / 20.0f;
		b = (float)i / 20.0f;
		App::DrawLine(sx, sy, ex, ey, r, g, b);
	}

	//------------------------------------------------------------------------
	// Example Triangle Drawing.
	//------------------------------------------------------------------------
	App::DrawTriangle(600.0f, 300.0f, -1, 1, 650.0f, 400.0f, 0, 1, 700.0f, 300.0f, 0, 1, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
	App::DrawTriangle(500.0f, 300.0f, 0, 1, 550.0f, 450.0f, 0.5, 1, 700.0f, 340.0f, -0.5, 1, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	App::DrawTriangle(800.0f, 300.0f, 0, 1, 850.0f, 400.0f, 0, 1, 900.0f, 300.0f, 0, 1, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, true);
}

//------------------------------------------------------------------------
// Add your shutdown code here. Called when the APP_QUIT_KEY is pressed.
// Just before the app exits.
//------------------------------------------------------------------------
void ExampleScene::destroyed()
{
	//------------------------------------------------------------------------
	// Example Sprite Code....
	delete testSprite;
	//------------------------------------------------------------------------
}