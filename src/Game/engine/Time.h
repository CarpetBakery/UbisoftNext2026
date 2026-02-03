#ifndef _TIME_H
#define _TIME_H

#include <AppSettings.h>

namespace Time
{
	constexpr float deltaTarget = 1.0f / APP_MAX_FRAME_RATE;
	
	// (0.0, 1.0)
	inline float delta = 0.0f;

	// Number of seconds since last frame
	inline float deltaSeconds = 0.0f;

	// Number of seconds since program started
	inline float seconds = 0.0f;

	// Return true on interval
	bool onInterval(float interval, float offset = 0.0f);
}

#endif // _TIME_H