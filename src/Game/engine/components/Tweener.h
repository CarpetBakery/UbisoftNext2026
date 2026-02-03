#ifndef _TWEENER_H
#define _TWEENER_H

#include <engine/ecs/System.h>
#include <functional>

// TODO: Look at my own implementation of Tweens from before
struct Tweener
{
	float currentTime = 0.0f;

	// How long the tween lasts
	float duration = 0.0f;
};


// What if I have a system that's a template?
// -> kind of a bad idea

//template <class T>
//class UpdateTweeners : public Engine::System
//{
//public:
//	void init() override;
//	void update() override;
//};

#endif // _TWEENER_H