#include "../LoadDialog.h"

#include <Content.h>

#include <engine/Ecs.h>

#include <engine/components/Animator.h>
#include <engine/components/Transform2D.h>
#include "../TritoneEditor.h"

using namespace Engine;
using namespace LoadDialogCommon;

void DrawLoadDialog::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<LoadDialog>());
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Animator>());

	setup(5, Type::Draw, sig);
}

void DrawLoadDialog::update()
{
	for (auto const &ent : m_entities)
	{
		auto &loadDialog = m_scene->getComponent<LoadDialog>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);
		auto &animator = m_scene->getComponent<Animator>(ent);

		Vec2i drawPosInit = transform.pos + loadOptionsOffset;
		Vec2i drawPos = drawPosInit;

		// Draw search path
		Graphics::drawText(transform.pos + Vec2i(105, 18),
			loadDialog.displayPath.c_str(),
			TritoneEditCommon::measureTextColor,
			GLUT_BITMAP_8_BY_13);

		int columnEntries = 0;
		for (auto &path : loadDialog.files)
		{
			std::string fileName = path.filename().string();

			// Shorten filename if needed
			if (fileName.size() > loadOptionMaxStringLength)
			{
				fileName = fileName.substr(0, loadOptionMaxStringLength - 3) + "...";
			}

			if (loadDialog.optionUnderMouse && loadDialog.optionUnderMouse == &path ||
				loadDialog.selectedOption && loadDialog.selectedOption == &path)
			{
				// Draw highlighted option
				Recti bg = Recti(drawPos - Vec2i(0, loadOptionsSize.y - 4), loadOptionsSize);
				Graphics::drawRectFilled(bg, TritoneEditCommon::measureTextColor);

				Graphics::drawText(drawPos, fileName, TritoneEditCommon::buttonBgColor, GLUT_BITMAP_9_BY_15);
			}
			else
			{
				// Draw regular-type option
				Graphics::drawText(drawPos, fileName, TritoneEditCommon::measureTextColor, GLUT_BITMAP_9_BY_15);
			}

			columnEntries++;
			drawPos.y += loadOptionsSize.y;

			if (columnEntries >= loadOptionsPerColumn)
			{
				columnEntries = 0;
				drawPos.x += loadOptionsSize.x;
				drawPos.y = drawPosInit.y;
			}
		}
	}
}