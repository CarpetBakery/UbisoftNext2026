#include "../LoadDialog.h"

#include <Content.h>

#include <engine/Ecs.h>

#include <engine/components/Animator.h>
#include <engine/components/Transform2D.h>
#include "../TritoneEditor.h"

using namespace Engine;
using namespace LoadDialogCommon;


void DrawSampleLoadDialog::init()
{
	Signature sig;
	sig.set(m_scene->getComponentType<LoadDialog>());
	sig.set(m_scene->getComponentType<SampleLoadDialog>());
	sig.set(m_scene->getComponentType<Transform2D>());
	sig.set(m_scene->getComponentType<Animator>());

	setup(6, Type::Draw, sig);
}

void DrawSampleLoadDialog::update()
{
	for (auto const &ent : m_entities)
	{
		auto &sampleLoadDialog = m_scene->getComponent<SampleLoadDialog>(ent);

		auto &tritone = m_scene->getComponent<TritoneEditor>(sampleLoadDialog.editorEntity);
		auto &noteGrid = m_scene->getComponent<NoteGrid>(sampleLoadDialog.editorEntity);

		auto &trackData = noteGrid.tracks.at(tritone.currentTrack);

		// Draw drum checkbox
		Content::sprCheckboxDrum.DrawEx(sampleLoadDialog.drumCheckboxPos, Vec2f(1, 1), trackData.oneshot);
	}
}