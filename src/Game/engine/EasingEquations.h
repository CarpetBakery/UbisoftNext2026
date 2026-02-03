#ifndef _EASING_EQUATIONS_H
#define _EASING_EQUATIONS_H

// https://github.com/godotengine/godot/blob/master/scene/animation/easing_equations.h
// https://robertpenner.com/easing/

namespace Engine
{
    namespace cubic
    {
        static float in(float time, float initial, float delta, float duration)
        {
            time /= duration;
            return delta * time * time * time + initial;
        }

        static float out(float time, float initial, float delta, float duration)
        {
            time = time / duration - 1;
            return delta * (time * time * time + 1) + initial;
        }

        static float in_out(float time, float initial, float delta, float duration)
        {
            time /= duration / 2;
            if (time < 1)
            {
                return delta / 2 * time * time * time + initial;
            }

            time -= 2;
            return delta / 2 * (time * time * time + 2) + initial;
        }

        static float out_in(float time, float initial, float delta, float duration)
        {
            if (time < duration / 2)
            {
                return out(time * 2, initial, delta / 2, duration);
            }
            float h = delta / 2;
            return in(time * 2 - duration, initial + h, h, duration);
        }
    }; // namespace cubic
}

#endif // _EASING_EQUATIONS_H