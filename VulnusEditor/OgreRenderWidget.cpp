
#include <OgreCamera.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreStringConverter.h>
#include <OgreViewport.h>

#include <QSettings>
#include <QTimer>
#include <QMouseEvent>

#include "OgreRenderWidget.h"

namespace Vulnus
{
	OgreRenderWidget::OgreRenderWidget( _In_ QWidget* parent, Qt::WindowFlags f )
		: QWidget(parent, f | Qt::MSWindowsOwnDC), isInit(false), renderWin(0),
			prevMousePos(0, 0), sceneMgr(0), inputListeners()
	{
		QPalette colourPalette = palette();
		colourPalette.setColor(QPalette::Active, QPalette::WindowText, Qt::black);
		colourPalette.setColor(QPalette::Active, QPalette::Window, Qt::black);
		setPalette(colourPalette);
	}

	OgreRenderWidget::~OgreRenderWidget()
	{
		if ( sceneMgr )
			sceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();
	}


	Ogre::RenderWindow* OgreRenderWidget::getRenderWindow() const
	{
		return renderWin;
	}

	Ogre::SceneManager* OgreRenderWidget::getSceneManager() const
	{
		return sceneMgr;
	}


	void OgreRenderWidget::initialise( const std::string &winName, HWND winID, _In_ const Ogre::NameValuePairList *miscParams )
	{
		//These attributes are the same as those use in a QGLWidget
		setAttribute(Qt::WA_PaintOnScreen);
		setAttribute(Qt::WA_NoSystemBackground);

		//Parameters to pass to Ogre::Root::createRenderWindow()
		Ogre::NameValuePairList params;
		params["useNVPerfHUD"] = "true";

		//If the user passed in any parameters then be sure to copy them into our own parameter set.
		//NOTE: Many of the parameters the user can supply (left, top, border, etc) will be ignored
		//as they are overridden by Qt. Some are still useful (such as FSAA).
		if ( miscParams )
			params.insert( miscParams->begin(), miscParams->end() );

		//The external windows handle parameters are platform-specific
		Ogre::String externalWindowHandleParams;

		//Accept input focus
		setFocusPolicy(Qt::StrongFocus);

		//positive integer for W32 (HWND handle) - According to Ogre Docs
		externalWindowHandleParams = Ogre::StringConverter::toString((unsigned int)(winId()));

		//Add the external window handle parameters to the existing params set.
		params["externalWindowHandle"] = externalWindowHandleParams;

		//Finally create our window.
		renderWin = Ogre::Root::getSingletonPtr()->createRenderWindow(winName, width(), height(), false, &params);

		sceneMgr = Ogre::Root::getSingletonPtr()->createSceneManager("DefaultSceneManager");

		setMouseTracking(true);
		isInit = true;
	}


	void OgreRenderWidget::addInputListener( _In_ InputListener *listener )
	{
		inputListeners.push_back(listener);
	}

	void OgreRenderWidget::startRendering()
	{
		if ( isInit )
		{
			if( !renderWin->isActive() )
				renderWin->setActive(true);

			Ogre::Root::getSingleton()._fireFrameStarted();
			renderWin->update();
			Ogre::Root::getSingleton()._fireFrameRenderingQueued();

			std::vector<InputListener*>::iterator it;
			for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
				(*it)->frameUpdate();

			Ogre::Root::getSingleton()._fireFrameEnded();

			QTimer::singleShot( 0, this, SLOT(startRendering()) );
		}
	}

	QPaintEngine *OgreRenderWidget::paintEngine() const
	{
		return 0;
	}

	void OgreRenderWidget::resizeEvent( _In_ QResizeEvent *evt )
	{
		if( renderWin )
		{
			renderWin->resize(width(), height());
			renderWin->windowMovedOrResized();

			for(int ct = 0; ct < renderWin->getNumViewports(); ++ct)
			{
				Ogre::Viewport* pViewport = renderWin->getViewport(ct);
				Ogre::Camera* pCamera = pViewport->getCamera();
				pCamera->setAspectRatio(static_cast<Ogre::Real>(pViewport->getActualWidth()) / static_cast<Ogre::Real>(pViewport->getActualHeight()));
			}
		}
	}

	void OgreRenderWidget::focusInEvent( _In_ QFocusEvent *evt )
	{
		const int NUM_KEY_STATES = 3;
		short keyStates[NUM_KEY_STATES] = { VK_MENU, VK_SHIFT, VK_CONTROL };
		int keyInt[NUM_KEY_STATES]      = { Qt::Key::Key_Alt, Qt::Key::Key_Shift, Qt::Key::Key_Control };
		
		for ( int i = 0; i < NUM_KEY_STATES; ++i )
		{
			bool altDown = GetKeyState(keyStates[i]) >> 7;

			QKeyEvent *pKeyEvt = 0;
			ButtonState bState = ButtonState::Up;
			if ( altDown )
			{
				pKeyEvt = new QKeyEvent( QEvent::Type::KeyPress, keyInt[i], Qt::KeyboardModifier::NoModifier );
				bState  = ButtonState::Down;
			} else
			{
				pKeyEvt = new QKeyEvent( QEvent::Type::KeyRelease, keyInt[i], Qt::KeyboardModifier::NoModifier );
			}
			std::vector<InputListener*>::iterator it;
			for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
				(*it)->keyboardEvent( pKeyEvt, bState );

			delete pKeyEvt;
		}
	}

	void OgreRenderWidget::mouseMoveEvent( _In_ QMouseEvent *evt )
	{
		std::vector<InputListener*>::iterator it;
		for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
			(*it)->mouseMoved( createMouseEvent(evt, ButtonState::None) );

		prevMousePos = Ogre::Vector2( evt->x(), evt->y() );
	}

	void OgreRenderWidget::mousePressEvent( _In_ QMouseEvent *evt )
	{
		prevMousePos = Ogre::Vector2( evt->x(), evt->y() );

		std::vector<InputListener*>::iterator it;
		for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
			(*it)->mouseEvent( createMouseEvent(evt, ButtonState::Down) );
	}

	void OgreRenderWidget::mouseReleaseEvent( _In_ QMouseEvent *evt )
	{
		std::vector<InputListener*>::iterator it;
		for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
			(*it)->mouseEvent( createMouseEvent(evt, ButtonState::Up) );
	}

	void OgreRenderWidget::wheelEvent( _In_ QWheelEvent *evt )
	{
		std::vector<InputListener*>::iterator it;
		for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
			(*it)->mouseMoved( createMouseEvent(evt) );
	}

	void OgreRenderWidget::keyPressEvent( _In_ QKeyEvent *evt )
	{
		//TODO: implement something like that these keyboard events are only passed when the viewport is selected

		std::vector<InputListener*>::iterator it;
		for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
			(*it)->keyboardEvent( evt, ButtonState::Down );
	}

	void OgreRenderWidget::keyReleaseEvent( _In_ QKeyEvent *evt )
	{
		std::vector<InputListener*>::iterator it;
		for ( it = inputListeners.begin(); it != inputListeners.end(); ++it )
			(*it)->keyboardEvent( evt, ButtonState::Up );
	}


	MouseEvent OgreRenderWidget::createMouseEvent( _In_ QWheelEvent *evt ) const
	{
		float normX = evt->x() / float( this->width() );
		float normY = evt->y() / float( this->height() );

		MouseEvent::Axis xAxis( evt->x(), prevMousePos.x - evt->x(), normX );
		MouseEvent::Axis yAxis( evt->y(), prevMousePos.y - evt->y(), normY );
		MouseEvent::Axis zAxis( evt->angleDelta().y(), evt->angleDelta().y(), 0.0f );

		return MouseEvent( xAxis, yAxis, zAxis, this->width(), this->height(), MouseEvent::Button::MiddleButton, ButtonState::None );
	}

	MouseEvent OgreRenderWidget::createMouseEvent( _In_ QMouseEvent *evt, ButtonState state ) const
	{
		float normX = evt->x() / float( this->width() );
		float normY = evt->y() / float( this->height() );

		MouseEvent::Axis xAxis( evt->x(), prevMousePos.x - evt->x(), normX );
		MouseEvent::Axis yAxis( evt->y(), prevMousePos.y - evt->y(), normY );
		MouseEvent::Axis zAxis( 0, 0, 0.0f );

		return MouseEvent( xAxis, yAxis, zAxis, this->width(), this->height(), evt->button(), state );
	}
}
