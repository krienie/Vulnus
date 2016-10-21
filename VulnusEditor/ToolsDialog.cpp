
#include <OgreCamera.h>
#include <OgreTechnique.h>

#include "Utils.h"
#include "VulnusEditor.h"
#include "ToolsDialog.h"

namespace Vulnus
{
	ToolsDialog::ToolsDialog( Renderer *ren, Ogre::Material *tessMat, QWidget *parent )
		: QDialog(parent), renderer(ren), tessellationMat(tessMat),
			doubleSpinBoxValueChanged(&QDoubleSpinBox::valueChanged)
	{
		ui.setupUi(this);

		tessFactorSliderChanged( ui.tessFactorSlider->value() );
		displacementScaleChanged( ui.displacementSpinbox->value() );

		QObject::connect( ui.buttonSolid, &QAbstractButton::clicked, this, &ToolsDialog::buttonSolidClicked );
		QObject::connect( ui.buttonWireframe, &QAbstractButton::clicked, this, &ToolsDialog::buttonWireframeClicked );
		QObject::connect( ui.cullmodeCheckbox, &QAbstractButton::toggled, this, &ToolsDialog::buttonCullmodeClicked );
		QObject::connect( ui.tessFactorSlider, &QAbstractSlider::valueChanged, this, &ToolsDialog::tessFactorSliderChanged );
		QObject::connect( ui.displacementSpinbox, doubleSpinBoxValueChanged, this, &ToolsDialog::displacementScaleChanged );
	}

	ToolsDialog::~ToolsDialog()
	{
	}


	bool ToolsDialog::isPaintingChecked() const
	{
		return ui.paintWoundCheckbox->isChecked();
	}

	void ToolsDialog::buttonSolidClicked( bool checked )
	{
		ui.buttonSolid->setChecked(true);
		ui.buttonWireframe->setChecked(false);
		ui.cullmodeCheckbox->setChecked(true);

		renderer->getCurrentCamera()->setPolygonMode( Ogre::PM_SOLID );
	}

	void ToolsDialog::buttonWireframeClicked( bool checked )
	{
		ui.buttonSolid->setChecked(false);
		ui.buttonWireframe->setChecked(true);
		ui.cullmodeCheckbox->setChecked(false);

		renderer->getCurrentCamera()->setPolygonMode( Ogre::PM_WIREFRAME );
	}

	void ToolsDialog::buttonCullmodeClicked( bool checked )
	{
		if ( !tessellationMat )
			return;

		if ( checked )
			tessellationMat->getTechnique(0)->getPass(0)->setCullingMode( Ogre::CULL_CLOCKWISE );
		else tessellationMat->getTechnique(0)->getPass(0)->setCullingMode( Ogre::CULL_NONE );
	}

	void ToolsDialog::tessFactorSliderChanged( int value )
	{
		std::stringstream ss;
		ss << value;
		ui.tessFactorLabel->setText( ss.str().c_str() );

		Ogre::GpuProgramParameters *gpuProgParams = Utils::getHullParameters("maleHand");
		gpuProgParams->setNamedConstant( "tessellationFactor", static_cast<float>(value) );
	}

	void ToolsDialog::displacementScaleChanged( double value )
	{
		Ogre::GpuProgramParameters *gpuProgParams = Utils::getDomainParameters("maleHand");
		gpuProgParams->setNamedConstant( "displacementScale", static_cast<float>(value) );
	}
}
