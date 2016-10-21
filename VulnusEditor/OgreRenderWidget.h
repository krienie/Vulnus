
#pragma once

#include <OgreRenderWindow.h>
#include <OgreSceneManager.h>
#include <OgreVector2.h>

#include <QWidget>

#include "InputListener.h"

namespace Vulnus
{
	class OgreRenderWidget : public QWidget
	{
		Q_OBJECT

	public slots:
		void startRendering();

	public:
		OgreRenderWidget( _In_ QWidget* parent = 0, Qt::WindowFlags f = 0 );
		~OgreRenderWidget();

		Ogre::RenderWindow* getRenderWindow() const;
		Ogre::SceneManager* getSceneManager() const;

		void initialise( const std::string &winName, HWND winID, _In_ const Ogre::NameValuePairList *miscParams = 0 );
		void addInputListener( _In_ InputListener *listener );

	protected:
		QPaintEngine *paintEngine() const;
		void paintEvent( _In_ QPaintEvent *evt ) { }
		void resizeEvent( _In_ QResizeEvent *evt );
		void focusInEvent( _In_ QFocusEvent *evt );
		void mouseMoveEvent( _In_ QMouseEvent *evt );
		void mousePressEvent( _In_ QMouseEvent *evt );
		void mouseReleaseEvent( _In_ QMouseEvent *evt );
		void wheelEvent( _In_ QWheelEvent *evt );
		void keyPressEvent( _In_ QKeyEvent *evt );
		void keyReleaseEvent( _In_ QKeyEvent *evt );

	private:
		MouseEvent createMouseEvent( _In_ QWheelEvent *evt ) const;
		MouseEvent createMouseEvent( _In_ QMouseEvent *evt, ButtonState state ) const;

		bool isInit;
		Ogre::RenderWindow* renderWin;
		Ogre::Vector2 prevMousePos;

		Ogre::SceneManager *sceneMgr;

		std::vector<InputListener*> inputListeners;
	};
}
