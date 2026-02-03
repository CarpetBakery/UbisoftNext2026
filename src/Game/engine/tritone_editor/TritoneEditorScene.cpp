#include "TritoneEditorScene.h"

#include "TritoneEditorFactory.h"
#include <engine/components/Transform2D.h>
#include <engine/components/Animator.h>
#include "components/TritoneEditor.h"
#include "components/NoteGrid.h"
#include "components/MouseCollider.h"
#include "components/Button.h"
#include "components/LoadDialog.h"

using namespace Engine;
namespace fs = std::filesystem;

namespace
{
    void menuHandler(int commandID)
    {
        switch (commandID)
        {
        case 2:
            break;
        case 3:
            MessageBox(
                NULL,
                "hello tritone",
                "ok",
                MB_OK | MB_ICONINFORMATION);
            break;
        }
    }

    void testMenu()
    {
        // Create test menu
        glutCreateMenu(menuHandler);

        glutAddMenuEntry("File", 1);
        glutAddMenuEntry("Edit", 2);
        glutAddMenuEntry("About", 3);

        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }
}

void TritoneEditorScene::init()
{
    // Make this scene draw at regular scale
    Graphics::resMult = 1.0f;

    // Register Components
    registerComponent<Transform2D>();
    registerComponent<TritoneEditor>();
    registerComponent<NoteGrid>();
    registerComponent<MouseCollider>();
    registerComponent<Animator>();
    registerComponent<Button>();
    registerComponent<LoadDialog>();
    registerComponent<SampleLoadDialog>();

    // Register Systems
    registerSystem<UpdateAndDrawAnimator>();

    registerSystem<UpdateTritone>();
    registerSystem<DrawTritone>();
    registerSystem<DrawNoteGrid>();

    registerSystem<UpdateLoadDialog>();
    registerSystem<DrawLoadDialog>();

    registerSystem<UpdateSampleLoadDialog>();
    registerSystem<DrawSampleLoadDialog>();

    registerSystem<UpdateButton>();
    // registerSystem<DrawMouseColliders>();


    // Populate scene
    Factory::createTritone(this);

    // Test creating right-click menu
    // testMenu();

    // Print out scene info
    printf("%s\n", toString().c_str());
}