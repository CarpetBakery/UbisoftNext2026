#include "Particles2D.h"

using namespace Engine;
using namespace GS;

void Particles2DCommon::destroy(Particles2D &particles)
{
	particles.oneShot = true;
}