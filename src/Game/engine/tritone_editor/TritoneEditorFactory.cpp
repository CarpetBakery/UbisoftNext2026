#include "TritoneEditorFactory.h"

#include <Content.h>

#include <engine/Input.h>

#include <engine/components/Animator.h>
#include <engine/components/Transform2D.h>
#include "components/MouseCollider.h"
#include "components/LoadDialog.h"

using namespace Engine;

Entity Factory::createTritone(Scene *scene)
{
	Entity ent = scene->createEntity();

	TritoneEditor tritone;
	NoteGrid noteGrid;

	// -- Create buttons --
	{
		Sprite &sprVel = Content::sprVelocityButton;
		Sprite &sprPan = Content::sprPanButton;
		Sprite &sprMenuButton = Content::sprMenuButton;
		Sprite &sprTrackButton = Content::sprTrackButton;
		Sprite &sprPlayButton = Content::sprPlayButton;

		tritone.velocityButton = createButton(
			scene, nullptr,
			Recti(
				Vec2i(0, 0), Vec2i(sprVel.GetWidth(), sprVel.GetHeight())),
			&sprVel, false);

		tritone.panButton = createButton(
			scene, nullptr,
			Recti(
				Vec2i(0, 0), Vec2i(sprPan.GetWidth(), sprPan.GetHeight())),
			&sprPan, false);

		tritone.menuButton = createButton(
			scene, nullptr,
			Recti(Vec2i(0, 0), Vec2i(sprMenuButton.GetWidth(), sprMenuButton.GetHeight())),
			&sprMenuButton);

		tritone.trackButton = createButton(
			scene, nullptr,
			Recti(Vec2i(0, 0), Vec2i(sprTrackButton.GetWidth(), sprTrackButton.GetHeight())),
			&sprTrackButton);

		tritone.playButton = createButton(
			scene, nullptr,
			Recti(Vec2i(0, 0), Vec2i(sprPlayButton.GetWidth(), sprPlayButton.GetHeight())),
			&sprPlayButton);

		// Setup button entities array
		tritone.buttonEntities = {
			tritone.velocityButton,
			tritone.panButton,
			tritone.menuButton,
			tritone.trackButton,
			tritone.playButton};
	}

	scene->addComponent(ent, tritone);
	scene->addComponent(ent, noteGrid);

	return ent;
}

Entity Factory::createButton(Scene *scene, Button::Callback pressedCallback, const Engine::Recti &rect, Engine::Sprite *sprite, bool showPressedAnim)
{
	Entity ent = scene->createEntity();

	Transform2D transform;

	MouseCollider mouseCollider;
	mouseCollider.rect = rect;

	Animator animator;
	animator.sprite = sprite;

	Button button;
	button.pressedCallback = pressedCallback;
	button.showPressedAnim = showPressedAnim;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, mouseCollider);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, button);

	return ent;
}

Entity Factory::createSongLoadDialog(Scene *scene, const std::filesystem::path &searchPath, LoadDialog::CloseCallback closedCallback, TritoneEditor &tritone)
{
	Entity ent = scene->createEntity();

	Transform2D transform;

	Animator animator;
	animator.sprite = &Content::sprLoadDialog;

	LoadDialog loadDialog;
	loadDialog.searchPath = searchPath;
	loadDialog.closedCallback = closedCallback;
	loadDialog.displayPath = "data/bgm/";
	loadDialog.requireDoubleClick = true;
	loadDialog.initialSelectedOption = tritone.saveFilepath;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, loadDialog);

	return ent;
}

Entity Factory::createLoadSampleDialog(Scene *scene,
											const std::filesystem::path &searchPath,
											LoadDialog::CloseCallback closedCallback,
											LoadDialog::SelectCallback selectCallback,
											const TritoneEditor &tritone, 
											const NoteGrid &noteGrid)
{
	Entity ent = scene->createEntity();

	Transform2D transform;

	Animator animator;
	animator.sprite = &Content::sprLoadSampleDialog;

	LoadDialog loadDialog;
	loadDialog.searchPath = searchPath;
	loadDialog.closedCallback = closedCallback;
	loadDialog.requireDoubleClick = true;
	loadDialog.selectCallback = selectCallback;
	loadDialog.initialSelectedOption = noteGrid.tracks.at(tritone.currentTrack).samplePath;
	loadDialog.displayPath = "data/bgm/sample/";

	SampleLoadDialog sampleLoadDialog;
	sampleLoadDialog.editorEntity = tritone.entityId;

	scene->addComponent(ent, transform);
	scene->addComponent(ent, animator);
	scene->addComponent(ent, loadDialog);
	scene->addComponent(ent, sampleLoadDialog);

	return ent;
}
