
#pragma once

#include <QtWidgets/QMainWindow>

#include "Renderer.h"
#include "WoundShapePainter.h"
#include "InputListener.h"
#include "ToolsDialog.h"
#include "WoundTimeline.h"

#include "ui_VulnusEditor.h"

namespace Vulnus
{
	class VulnusEditor : public QMainWindow, public InputListener
	{
		Q_OBJECT

	public slots:
		void timeSliderChanged( int value );
		void openToolsMenu( QAction *action );

	public:
		VulnusEditor( QWidget *parent = 0 );
		~VulnusEditor();

		void frameUpdate( /*const Ogre::FrameEvent& e*/ );
		void mouseMoved( const MouseEvent &evt );
		void mouseEvent( const MouseEvent &evt );
		void keyboardEvent( _In_ QKeyEvent *evt, const ButtonState &state );

	private:
		void loadWoundScene();

		Ui::VulnusEditorClass ui;

		ToolsDialog *toolsWin;

		Renderer *renderer;
		WoundShapePainter *painter;
		WoundTimeline woundTimeline;

		Ogre::Material *maleHandMaterial;
		const std::string HAND_ALBEDO_TEXSTATE;
		const std::string HAND_NORMAL_TEXSTATE;
		const std::string HAND_REFLECT_TEXSTATE;
		const std::string HAND_HEIGHT_TEXSTATE;

		bool leftAltDown;
		bool leftMouseDown;
	};
}
