#include "Time.h"

// Thanks again to Noel Berry's Blah framework for being a fantastic resource
// https://github.com/NoelFB/blah/blob/master/include/blah_time.h

bool Time::onInterval(float interval, float offset)
{
	auto last = static_cast<long>((Time::seconds - offset - Time::deltaSeconds) / interval);
	auto next = static_cast<long>((Time::seconds - offset) / interval);
	return last < next;
}