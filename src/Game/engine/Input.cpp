#include "Input.h"
#include <app.h>

#include <engine/ecs/Scene.h>
#include <engine/Game.h>

using namespace Engine;
using namespace Input;

namespace
{
    // Original array doesn't initialize all elements, and the original "GlutMouse"
    // function in main.cpp actually writes past the end of the array when you
    // scroll the mouse wheel...
    int mouseButtonState[5] = {GLUT_UP, GLUT_UP, GLUT_UP, 0, 0};
    constexpr int GLUT_WHEEL_UP = 3;
    constexpr int GLUT_WHEEL_DOWN = 4;

    void glutMouse(int button, int state, int x, int y)
    {
        if (button < GLUT_WHEEL_UP)
        {
            mouseButtonState[button] = state;
        }
        else if (state == GLUT_DOWN)
        {
            // Only listen to GLUT_DOWN events on mouse wheel
            // Increment every time we scroll on the mouse wheel
            mouseButtonState[button]++;
        }
        // printf("button: %d, state: %d - (%d, %d)\n", button, state, x, y);
    }

    bool isMouseButtonPressed(int button)
    {
        return mouseButtonState[button] == GLUT_DOWN;
    }

    int getGlutMouseState(int button)
    {
        return mouseButtonState[button];
    }
}

void Manager::init(Game *game)
{
    m_game = game;

    // Hijack glut mouse events
    glutMouseFunc(glutMouse);

    // Setup keyboard states
    memset(&keyboardState, 0, KeyboardState::size);
    memset(&keyboardStatePressed, 0, KeyboardState::size);
    memset(&keyboardStatePrev, 0, KeyboardState::size);
}

void Manager::update()
{
    // -- Keyboard --
    for (int i = 0; i < KeyboardState::size; i++)
    {
        keyboardState.key[i] = App::IsKeyPressed(static_cast<App::Key>(i));

        keyboardStatePressed.key[i] = keyboardState.key[i] && !keyboardStatePrev.key[i];
        
        keyboardStatePrev.key[i] = keyboardState.key[i];
    }

    // -- Mouse --
    // Get current state of all mouse buttons
    _mCurrent = {
        isMouseButtonPressed(GLUT_LEFT_BUTTON),
        isMouseButtonPressed(GLUT_RIGHT_BUTTON),
        isMouseButtonPressed(GLUT_MIDDLE_BUTTON),
    };

    // Get mouse pressed state
    mPressedState.left = _mCurrent.left && !_mPrev.left;
    mPressedState.right = _mCurrent.right && !_mPrev.right;
    mPressedState.middle = _mCurrent.middle && !_mPrev.middle;

    wState.up = getGlutMouseState(GLUT_WHEEL_UP);
    wState.down = getGlutMouseState(GLUT_WHEEL_DOWN);

    _mPrev = _mCurrent;

    // Reset wheel state
    mouseButtonState[GLUT_WHEEL_UP] = 0;
    mouseButtonState[GLUT_WHEEL_DOWN] = 0;

    // Set previous mouse position
    _mousePosPrev = mousePos();
}

bool Manager::key(const App::Key key) const
{
    return keyboardState.key[static_cast<int>(key)];
}

bool Manager::keyPressed(const App::Key key) const
{
    return keyboardStatePressed.key[static_cast<int>(key)];
}

Vec2i Manager::mousePosPrev() const
{
    return _mousePosPrev;
}

Vec2i Manager::mousePos() const
{
    Vec2i camPos = m_game->getScene()->m_camera.m_pos;
    return mousePosScreen() + camPos;
}

Vec2i Engine::Input::Manager::mousePosScreen() const
{
    float mx, my;
    App::GetMousePos(mx, my);
    return Vec2i(mx, my);
}

bool Engine::Input::Manager::mButton(Mouse btn) const
{
    switch (btn)
    {
    case Mouse::Left:
        return mLeft();

    case Mouse::Right:
        return mRight();

    case Mouse::Middle:
        return mMiddle();
    }
    return false;
}

bool Manager::mLeft() const
{
    return _mCurrent.left;
}

bool Manager::mRight() const
{
    return _mCurrent.right;
}

bool Manager::mMiddle() const
{
    return _mCurrent.middle;
}

bool Engine::Input::Manager::mButtonPressed(Mouse btn) const
{
    switch (btn)
    {
    case Mouse::Left:
        return mLeftPressed();

    case Mouse::Right:
        return mRightPressed();

    case Mouse::Middle:
        return mMiddlePressed();
    }
    return false;
}

bool Manager::mLeftPressed() const
{
    // It seems like the initial mouse button state from the API is unreliable
    // For some reason it thinks I'm pressing MOUSE2 until I actually press it
    // then things work as expected
    return mPressedState.left;
}

bool Manager::mRightPressed() const
{
    return mPressedState.right;
}

bool Manager::mMiddlePressed() const
{
    return mPressedState.middle;
}

int Manager::mWheelUp() const
{
    return wState.up;
}

int Manager::mWheelDown() const
{
    return wState.down;
}

Vec2f Manager::getAxis(const App::Key left, const App::Key right, const App::Key up, const App::Key down) const
{
    return Vec2f(
            static_cast<float>(key(right)) - static_cast<float>(key(left)),
            static_cast<float>(key(down)) - static_cast<float>(key(up)));
}