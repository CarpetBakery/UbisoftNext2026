#include "Audio.h"

#include <vendor/miniaudio/miniaudio.h>
//#include <SimpleSound.h>
#include <app.h>

using namespace Engine;

void Audio::play(const std::filesystem::path &path, bool cutPrev, bool looping)
{
	if (cutPrev)
	{
		// Only allow one sound at a time
		stop(path);
	}

	App::PlayAudio(path.string().c_str(), looping);
}

void Audio::stop(const std::filesystem::path &path)
{
	App::StopAudio(path.string().c_str());
}
