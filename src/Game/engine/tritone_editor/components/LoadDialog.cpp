#include "LoadDialog.h"

using namespace Engine;

void LoadDialogCommon::submitLoadDialog(LoadDialog &loadDialog, const Entity &ent, const std::filesystem::path &path)
{
	bool cancelled = false;
	if (path == "")
	{
		cancelled = true;
	}

	if (loadDialog.closedCallback)
	{
		loadDialog.closedCallback(cancelled, path);
	}
}
