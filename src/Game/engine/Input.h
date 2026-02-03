#ifndef _INPUT_H
#define _INPUT_H

#include "Spatial.h"
#include <app.h>

namespace Engine
{
	class Game;

	namespace Input
	{
		enum class Mouse
		{
			Left = GLUT_LEFT_BUTTON,
			Middle = GLUT_MIDDLE_BUTTON,
			Right = GLUT_RIGHT_BUTTON,
		};

		// Input manager (used by game class to keep track of input states)
		class Manager
		{
			friend class Game;

		private:
			// Keyboard internal
			struct KeyboardState
			{
				static inline const size_t size = App::Key::KEY_COUNT;
				bool key[size];
			};

			KeyboardState keyboardState, keyboardStatePressed, keyboardStatePrev;

			// Mouse internal
			struct MouseState
			{
				bool left = false;
				bool right = false;
				bool middle = false;
			};

			struct WheelState
			{
				int up = 0;
				int down = 0;
			};

			// Reference back to the game class
			Game *m_game = nullptr;

			MouseState mPressedState;
			MouseState _mCurrent, _mPrev;
			WheelState wState;
			Vec2i _mousePosPrev = Vec2i(0, 0);

			void init(Game *game);
			void update();

		public:
			// -- Keyboard input --
			bool key(const App::Key key) const;
			bool keyPressed(const App::Key key) const;

			// -- Mouse input --
			// The mouse position on the previous frame
			Vec2i mousePosPrev() const;
			// The current mouse position inside the window
			Vec2i mousePos() const;

			// Ignore camera offsets and just get the mouse position onscreen
			Vec2i mousePosScreen() const;

			// Is a mouse button held?
			bool mButton(Mouse btn) const;
			// Is the left mouse button held?
			bool mLeft() const;
			// Is the right mouse button held?
			bool mRight() const;
			// Is the middle mouse button held?
			bool mMiddle() const;

			// Was a mouse button just pressed?
			bool mButtonPressed(Mouse btn) const;
			// Was the left mouse button just pressed?
			bool mLeftPressed() const;
			// Was the right mouse button just pressed?
			bool mRightPressed() const;
			// Was the middle mouse button just pressed?
			bool mMiddlePressed() const;

			// The number of times the mouse wheel has scrolled up this frame
			int mWheelUp() const;
			// The number of times the mouse wheel has scrolled down this frame
			int mWheelDown() const;

			// Get input axis based on four buttons
			Vec2f getAxis(const App::Key left, const App::Key right, const App::Key up, const App::Key down) const;
		};

	}
}

#endif // _INPUT_H