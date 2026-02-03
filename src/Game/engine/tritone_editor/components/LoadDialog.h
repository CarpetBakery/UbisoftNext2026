#ifndef _LOAD_DIALOG_H
#define _LOAD_DIALOG_H

#include <engine/Spatial.h>
#include <engine/ecs/System.h>

#include <filesystem>
#include <functional>
#include <vector>

// An in-game "load file" dialog. Used in my editor tools

struct LoadDialog
{
	// The path to search in
	std::filesystem::path searchPath;

	// The path displayed at the top of the dialog
	std::string displayPath;

	// The files we found at the searchPath
	std::vector<std::filesystem::path> files;

	// The option currently under the mouse
	std::filesystem::path* optionUnderMouse = nullptr;

	// Called when the dialog is closed
	using CloseCallback = std::function<void(bool cancelled, const std::filesystem::path &path)>;
	CloseCallback closedCallback;

	// -- Double click mode --
	// If enabled, user will need to click once to select an option, and then again to submit it
	bool requireDoubleClick = false;

	// The option currently selected (unused if in single click mode)
	std::filesystem::path* selectedOption = nullptr;

	// Used to show the instrument currently selected when the dialog is created
	std::filesystem::path initialSelectedOption = "";

	// Called when an option is selected (unused if in single click mode)
	using SelectCallback = std::function<void(const std::filesystem::path& path)>;
	SelectCallback selectCallback;
};

struct SampleLoadDialog
{
	// A dialog with load sample controls
	Engine::Entity editorEntity;
	
	Engine::Vec2i drumCheckboxPos = Engine::Vec2i(0, 0);
};

class UpdateLoadDialog: public Engine::System
{
public:
	void init() override;
	void entityAdded(Engine::Entity const& ent) override;
	void update() override;

	void handleClick(const Engine::Entity &ent, LoadDialog& loadDialog, std::filesystem::path &path);
};

class DrawLoadDialog : public Engine::System
{
public:
	void init() override;
	void update() override;
};

class UpdateSampleLoadDialog : public Engine::System
{
public:
	void init() override;
	void entityAdded(Engine::Entity const& ent) override;
	void update() override;
};

class DrawSampleLoadDialog : public Engine::System
{
public:
	void init() override;
	void update() override;
};

namespace LoadDialogCommon
{
	constexpr Engine::Vec2i loadOptionsOffset = Engine::Vec2i(19, 44);
	constexpr Engine::Vec2i loadOptionsSize = Engine::Vec2i(150, 17);
	constexpr int loadOptionsPerColumn = 17;
	constexpr int loadOptionMaxStringLength = 15;

	// constexpr Engine::Vec2i searchPathOffset = Engine::Vec2i(0, 0);

	constexpr Engine::Vec2i drumCheckOffset = Engine::Vec2i(500, 8);

	void submitLoadDialog(LoadDialog& loadDialog, const Engine::Entity& ent, const std::filesystem::path &path = "");
}


#endif // _LOAD_DIALOG_H