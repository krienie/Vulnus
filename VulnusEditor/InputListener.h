
#pragma once

#ifndef OIS_DYNAMIC_LIB
	#define OIS_DYNAMIC_LIB
#endif

#include <OgreFrameListener.h>

#include <QMouseEvent>
#include <QKeyEvent>

namespace Vulnus
{
	enum ButtonState
	{
		Up,
		Down,
		None,
		NumButtonStates
	};

	class MouseEvent
	{
		public:
			typedef Qt::MouseButton Button;

			struct Axis
			{
				public:
					Axis( int _abs, int _rel, float _norm )
						: absolute(_abs), relative(_rel), normalized(_norm) { }

					int abs() const { return absolute; }
					int rel() const { return relative; }
					float norm() const { return normalized; }

				private:
					int absolute;
					int relative;
					float normalized;
			};

			MouseEvent( Axis _xAxis, Axis _yAxis, Axis _zAxis, int _width, int _height, Button _button, ButtonState _state )
				: xAxis(_xAxis), yAxis(_yAxis), zAxis(_zAxis), w(_width), h(_height), b(_button), s(_state) { }
			//~MouseEvent();

			Axis X() const { return xAxis; }
			Axis Y() const { return yAxis; }
			Axis Z() const { return zAxis; }
			int width() const { return w; }
			int height() const { return h; }
			Button button() const { return b; }
			ButtonState state() const { return s; }

		private:
			Axis xAxis;
			Axis yAxis;
			Axis zAxis;
			int w;
			int h;
			Button b;
			ButtonState s;
	};

	class InputListener
	{
		public:
			virtual void frameUpdate( /*const Ogre::FrameEvent& e*/ ) { }
			virtual void mouseMoved( const MouseEvent &evt ) { }
			virtual void mouseEvent( const MouseEvent &evt ) { }
			//TODO: create custom KeyEvent maybe?
			virtual void keyboardEvent( _In_ QKeyEvent *evt, const ButtonState &state ) { }

		protected:
			InputListener() { }
			virtual ~InputListener() { }

		private:
	};
}
