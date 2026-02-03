#include "../LoadDialog.h"

#include <Content.h>

#include <engine/Ecs.h>
#include <engine/Input.h>

#include "../TritoneEditor.h"
#include <engine/components/Animator.h>
#include <engine/components/Transform2D.h>

using namespace Engine;
using namespace LoadDialogCommon;

void UpdateLoadDialog::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<LoadDialog>());
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Animator>());
	setup(0, Type::Update, sig);
}

void UpdateLoadDialog::entityAdded(Entity const &ent)
{
	auto &loadDialog = m_scene->getComponent<LoadDialog>(ent);
	auto &transform = m_scene->getComponent<Transform2D>(ent);
	auto &animator = m_scene->getComponent<Animator>(ent);

	// Setup dialog pos
	transform.pos = (m_game->getScreenSize() / 2) - (animator.sprite->GetSize() / 2);

	// Get all files in search path
	loadDialog.files.clear();
	for (auto &entry : std::filesystem::directory_iterator(loadDialog.searchPath))
	{
		if (entry.is_directory())
		{
			continue;
		}
		const std::filesystem::path &entryPath = entry.path();

		loadDialog.files.push_back(entryPath);

		// Highlight the instrument that's loaded into this track
		if (entryPath.filename() == loadDialog.initialSelectedOption.filename())
		{
			loadDialog.selectedOption = &loadDialog.files.back();
		}
	}
}

void UpdateLoadDialog::update()
{
	for (auto const &ent : m_entities)
	{
		auto &loadDialog = m_scene->getComponent<LoadDialog>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);
		auto &animator = m_scene->getComponent<Animator>(ent);

		// Cancel dialog
		if (m_game->m_input.keyPressed(App::KEY_ESC))
		{
			LoadDialogCommon::submitLoadDialog(loadDialog, ent);
			m_scene->queueDestroy(ent);
			return;
		}

		loadDialog.optionUnderMouse = nullptr;

		// Check if we were just clicking outside the dialog to exit
		if (m_game->m_input.mLeftPressed())
		{
			Recti dialogArea = Recti(transform.pos, animator.sprite->GetSize());
			if (!dialogArea.contains(m_game->m_input.mousePos()))
			{
				LoadDialogCommon::submitLoadDialog(loadDialog, ent);
				m_scene->queueDestroy(ent);
				return;
			}
		}

		// Check if mouse is hovering over any file
		Vec2i checkPosInit = transform.pos + loadOptionsOffset;
		Vec2i checkPos = checkPosInit;
		int columnEntries = 0;

		for (auto &path : loadDialog.files)
		{
			Recti collider = Recti(checkPos - Vec2i(0, loadOptionsSize.y - 4), loadOptionsSize);

			if (collider.contains(m_game->m_input.mousePos()))
			{
				loadDialog.optionUnderMouse = &path;

				if (m_game->m_input.mLeftPressed())
				{
					handleClick(ent, loadDialog, path);
					return;
				}
				break;
			}

			columnEntries++;
			checkPos.y += loadOptionsSize.y;

			if (columnEntries >= loadOptionsPerColumn)
			{
				columnEntries = 0;
				checkPos.x += loadOptionsSize.x;
				checkPos.y = checkPosInit.y;
			}
		}

		// Move options with arrow keys
		if (loadDialog.requireDoubleClick)
		{
			if (m_game->m_input.keyPressed(App::KEY_UP))
			{
				if (!loadDialog.selectedOption)
				{
					// Select an option first
					handleClick(ent, loadDialog, loadDialog.files.front());
				}
				else if (loadDialog.selectedOption > &loadDialog.files.front())
				{
					handleClick(ent, loadDialog, *(loadDialog.selectedOption - 1));
				}
			}
			else if (m_game->m_input.keyPressed(App::KEY_DOWN))
			{
				if (!loadDialog.selectedOption)
				{
					// Select an option first
					handleClick(ent, loadDialog, loadDialog.files.front());
				}
				else if (loadDialog.selectedOption < &loadDialog.files.back())
				{
					handleClick(ent, loadDialog, *(loadDialog.selectedOption + 1));
				}
			}

			// Select with enter
			if (m_game->m_input.keyPressed(App::KEY_ENTER) && loadDialog.selectedOption)
			{
				loadDialog.optionUnderMouse = loadDialog.selectedOption;
				handleClick(ent, loadDialog, *loadDialog.selectedOption);
				return;
			}
		}
	}
}

void UpdateLoadDialog::handleClick(const Entity &ent, LoadDialog &loadDialog, std::filesystem::path &path)
{
	if (!loadDialog.requireDoubleClick)
	{
		// Select this file
		LoadDialogCommon::submitLoadDialog(loadDialog, ent, path);
		m_scene->queueDestroy(ent);
	}
	else
	{
		if (loadDialog.selectedOption != &path)
		{
			// Set this new option to be selected
			loadDialog.selectedOption = &path;

			if (loadDialog.selectCallback)
			{
				loadDialog.selectCallback(path);
			}
		}
		else
		{
			// Select this option
			LoadDialogCommon::submitLoadDialog(loadDialog, ent, path);
			m_scene->queueDestroy(ent);
		}
	}
}