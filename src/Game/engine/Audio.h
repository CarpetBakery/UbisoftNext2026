#ifndef _AUDIO_H
#define _AUDIO_H

#include <filesystem>

namespace Engine
{
	namespace Audio
	{
		// Play a sound. Optionally, loop it forever
		void play(const std::filesystem::path &path, bool cutPrev = true, bool looping = false);
		// Stop playing a sound if we are playing it
		void stop(const std::filesystem::path &path);

		// Can't do these because CSimpleSound doesn't provide them :(
		// Change a sound's volume
		//void setGain(const std::filesystem::path &path, float gain);
		// Load sound if it isn't already loaded. Otherwise, do nothing
		//void loadSound(const std::filesystem::path &path);
	}
}

#endif // _AUDIO_H