
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreTechnique.h>
#include <OgreD3D11TextureManager.h>
#include <OgreViewport.h>

#include "NormalmapGen.h"
#include "NoiseRendererGPU.h"
#include "Utils.h"
#include "VulnusEditor.h"

namespace Vulnus
{
	VulnusEditor::VulnusEditor(QWidget *parent)
	: QMainWindow(parent), toolsWin(0), renderer(0), painter(0), woundTimeline(),
		maleHandMaterial(0), leftAltDown(false), leftMouseDown(false),
			HAND_ALBEDO_TEXSTATE("handAlbedoTexState"), HAND_NORMAL_TEXSTATE("handNormalTexState"),
			HAND_REFLECT_TEXSTATE("handReflectTexState"), HAND_HEIGHT_TEXSTATE("handHeightTexState")
	{
		ui.setupUi(this);

		// setup Ogre render scene
		renderer = new Vulnus::Renderer( HWND(this->winId()), ui.centralWidget );
		renderer->getMainRenderWidget()->setGeometry( ui.mainViewLabel->geometry() );

		renderer->setupScene();
		renderer->getMainRenderWidget()->addInputListener(this);

		Ogre::MaterialPtr tessMatPtr = Ogre::MaterialManager::getSingletonPtr()->getByName("maleHand");
		toolsWin = new ToolsDialog(renderer, tessMatPtr.get(), this);
		toolsWin->hide();
		tessMatPtr.setNull();

		loadWoundScene();

		renderer->getMainRenderWidget()->startRendering();
	}

	VulnusEditor::~VulnusEditor()
	{
		if ( toolsWin )
		{
			toolsWin->close();
			delete toolsWin;
		}

		if ( painter )
			delete painter;
		delete renderer;
	}


	void VulnusEditor::loadWoundScene()
	{
		Ogre::Entity *hand  = Renderer::loadModel( "hand", "maleHandTangents.mesh", renderer->getMainRenderWidget()->getSceneManager() );
		Ogre::Entity *nails = Renderer::loadModel( "nails", "maleHandNailsTangents.mesh", renderer->getMainRenderWidget()->getSceneManager() );
		
		Renderer::attachMaterial( nails, "maleHandNails" );
		maleHandMaterial = Renderer::attachMaterial( hand, "maleHand" );

		// set light shader constants
		Ogre::SceneManager *mainSceneMgr = renderer->getMainRenderWidget()->getSceneManager();		
		Ogre::ColourValue ambientLight   = mainSceneMgr->getAmbientLight();
		Ogre::Light *mainDirLight        = mainSceneMgr->getLight("MainDirLight");

		// hand mesh
		Ogre::GpuProgramParameters *gpuProgParams = Utils::getVertexParameters("maleHand");
		gpuProgParams->setNamedConstant( "dLightDir", mainDirLight->getDirection() );
		gpuProgParams = Utils::getFragmentParameters("maleHand");
		gpuProgParams->setNamedConstant( "dLightIntensity", mainDirLight->getPowerScale() );
		gpuProgParams->setNamedConstant( "aLightClr", ambientLight );

		// nails mesh
		gpuProgParams = Utils::getVertexParameters("maleHandNails");
		gpuProgParams->setNamedConstant( "dLightDir", mainDirLight->getDirection() );
		gpuProgParams = Utils::getFragmentParameters("maleHandNails");
		gpuProgParams->setNamedConstant( "dLightIntensity", mainDirLight->getPowerScale() );
		gpuProgParams->setNamedConstant( "aLightClr", ambientLight );

		// setup texture painter
		Ogre::Texture *shapeUpperSkinTex = Renderer::getManualTexture( "woundShapeUpperSkinTexture", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *shapeLowerSkinTex = Renderer::getManualTexture("woundShapeLowerSkinTexture", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		painter = new Vulnus::WoundShapePainter( renderer, shapeUpperSkinTex, shapeLowerSkinTex );

		// setup woundTimeline
		Ogre::D3D11RenderSystem *dxRenderSys = dynamic_cast<Ogre::D3D11RenderSystem*>( Ogre::Root::getSingletonPtr()->getRenderSystem() );
		woundTimeline.initialize( dxRenderSys->_getDevice().get(), renderer, painter->getUVRecordSinkPtr() );
		MultiRTTSampler::SampledTexture sampledTex = woundTimeline.sample( ui.timelineSlider->value() );


		// connect textures to the hand material
		Ogre::TextureUnitState *texState = maleHandMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( sampledTex.albedo->getName() );
		texState->setName( HAND_ALBEDO_TEXSTATE );
		texState = maleHandMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( sampledTex.normal->getName() );
		texState->setName( HAND_NORMAL_TEXSTATE );
		texState = maleHandMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( sampledTex.reflection->getName() );
		texState->setName( HAND_REFLECT_TEXSTATE );
		texState = maleHandMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( sampledTex.height->getName() );
		texState->setName( HAND_HEIGHT_TEXSTATE );

		// allow textures to be bound to domain shaders and pixel shaders
		dxRenderSys->_setBindingType( Ogre::TextureUnitState::BT_TESSELLATION_DOMAIN | Ogre::TextureUnitState::BT_FRAGMENT );

		// set static tessellation factor
		gpuProgParams = Utils::getHullParameters("maleHand");
		gpuProgParams->setNamedConstant( "tessellationFactor", 19.0f );


		QObject::connect( ui.timelineSlider, &QAbstractSlider::valueChanged, this, &VulnusEditor::timeSliderChanged );
		QObject::connect( ui.menuTools, &QMenu::triggered, this, &VulnusEditor::openToolsMenu );
	}

	void VulnusEditor::timeSliderChanged( int value )
	{
		if ( woundTimeline.isInitialized() )
		{
			// calculate normalized position on timeline
			int sliderRange   = ui.timelineSlider->maximum() - ui.timelineSlider->minimum();
			float timelinePos = value / float(sliderRange);

			MultiRTTSampler::SampledTexture sampledTex = woundTimeline.sample( timelinePos );

			// connect textures to the hand material
			Ogre::TextureUnitState *texState = maleHandMaterial->getTechnique(0)->getPass(0)->getTextureUnitState( HAND_ALBEDO_TEXSTATE );
			//texState->setTextureName( sampledTex.albedo->getName() );
			texState = maleHandMaterial->getTechnique(0)->getPass(0)->getTextureUnitState( HAND_NORMAL_TEXSTATE );
			texState->setTextureName( sampledTex.normal->getName() );
			texState = maleHandMaterial->getTechnique(0)->getPass(0)->getTextureUnitState( HAND_REFLECT_TEXSTATE );
			texState->setTextureName( sampledTex.reflection->getName() );
			texState = maleHandMaterial->getTechnique(0)->getPass(0)->getTextureUnitState( HAND_HEIGHT_TEXSTATE );
			texState->setTextureName( sampledTex.height->getName() );
		}
	}

	void VulnusEditor::openToolsMenu( QAction *action )
	{
		toolsWin->show();
	}


	void VulnusEditor::frameUpdate( /*const Ogre::FrameEvent& e*/ )
	{
	}


	void VulnusEditor::mouseMoved( const MouseEvent &evt )
	{
		if ( toolsWin->isPaintingChecked() && leftMouseDown && !leftAltDown && painter )
			painter->paint( evt.X().norm(), evt.Y().norm() );
	}


	void VulnusEditor::mouseEvent( const MouseEvent &evt )
	{
		if ( evt.button() == MouseEvent::Button::LeftButton )
		{
			leftMouseDown = evt.state() == ButtonState::Down;

			if ( painter && toolsWin->isPaintingChecked() && !leftAltDown )
			{
				if (  leftMouseDown )
				{
					painter->beginUVRecord( evt.X().norm(), evt.Y().norm() );
				} else
				{
					clock_t tStart = clock();
					std::cout << "starting rendering key positions" << std::endl;

					// paint wound patch
					painter->end();
					painter->clearTextures();
					painter->paintPatch();

					// render timeline
					if ( woundTimeline.isInitialized() )
					{
						woundTimeline.updateFrames();
						timeSliderChanged( ui.timelineSlider->value() );
					}

					double time = double( clock() - tStart ) / (double)CLOCKS_PER_SEC;
					std::cout << "done in: " << time << " seconds" << std::endl;
					std::cout << "----------------------------------------" << std::endl;

					return;
				}
			}
		}
	}


	void VulnusEditor::keyboardEvent( _In_ QKeyEvent *evt, const ButtonState &state )
	{
		if ( evt->key() == Qt::Key::Key_Alt )
			leftAltDown = state == ButtonState::Down;
	}
}
