#include "Camera.h"

using namespace Engine;

Recti Engine::Camera::getRect() const
{
	return Recti(m_pos, m_size);
}

void Engine::Camera::shake(float amt)
{
	m_shakeAmp = amt;
}
