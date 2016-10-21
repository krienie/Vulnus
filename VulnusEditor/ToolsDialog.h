
#pragma once

#include <OgreMaterial.h>
#include <QtWidgets/QDialog>

#include "ui_ToolsDialog.h"

#include "Renderer.h"

namespace Vulnus
{
	class ToolsDialog : public QDialog
	{
		Q_OBJECT

	public slots:
		void buttonSolidClicked( bool checked = false );
		void buttonWireframeClicked( bool checked = false );
		void buttonCullmodeClicked( bool checked = false );
		void tessFactorSliderChanged( int value );
		void displacementScaleChanged( double value );

	public:
		ToolsDialog( Renderer *ren, Ogre::Material *tessMat, QWidget *parent = 0 );
		~ToolsDialog();

		bool isPaintingChecked() const;

		void (QDoubleSpinBox:: *doubleSpinBoxValueChanged)(double);

	private:
		Ui::Tools ui;

		Renderer *renderer;
		Ogre::Material *tessellationMat;
	};
}
