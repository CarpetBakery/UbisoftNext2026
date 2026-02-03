#include "../LoadDialog.h"

#include <Content.h>

#include <engine/Ecs.h>
#include <engine/Input.h>

#include <engine/components/Animator.h>
#include <engine/components/Transform2D.h>
#include "../TritoneEditor.h"

using namespace Engine;
using namespace LoadDialogCommon;

void UpdateSampleLoadDialog::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<LoadDialog>());
	sig.set(m_scene->getComponentType<SampleLoadDialog>());
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Animator>());

	setup(1, Type::Update, sig);
}

void UpdateSampleLoadDialog::entityAdded(Entity const &ent)
{
}

void UpdateSampleLoadDialog::update()
{
	for (auto const &ent : m_entities)
	{
		auto &loadDialog = m_scene->getComponent<LoadDialog>(ent);
		auto &transform = m_scene->getComponent<Transform2D>(ent);
		auto &animator = m_scene->getComponent<Animator>(ent);
		auto &sampleLoadDialog = m_scene->getComponent<SampleLoadDialog>(ent);

		auto &tritone = m_scene->getComponent<TritoneEditor>(sampleLoadDialog.editorEntity);
		auto &noteGrid = m_scene->getComponent<NoteGrid>(sampleLoadDialog.editorEntity);

		auto &trackData = noteGrid.tracks.at(tritone.currentTrack);

		sampleLoadDialog.drumCheckboxPos = transform.pos + drumCheckOffset;

		if (m_game->m_input.mLeftPressed())
		{
			// Check if we clicked on button
			Recti collider = Recti(sampleLoadDialog.drumCheckboxPos, Content::sprCheckboxDrum.GetSize());

			if (collider.contains(m_game->m_input.mousePos()))
			{
				trackData.oneshot = !trackData.oneshot;

				// Apply to playback engine
				m_game->m_tritonePlayer.loadFromEditor(tritone, noteGrid);
			}
		}
	}
}