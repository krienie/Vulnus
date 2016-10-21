
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreTechnique.h>

#include "Renderer.h"
#include "GradientEdgeRenderer.h"
#include "NoiseRendererGPU.h"
#include "SharedResources.h"
#include "PreScabFrame.h"

namespace Vulnus
{
	PreScabFrame::PreScabFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model )
		: Keyframe("PreScabFrame", "preScabFrameAlbedo", "preScabFrameNormal",
						"preScabFrameReflection", "preScabFrameHeight", camera, model),
			noiseSeed( NoiseRendererGPU::getNewSeed() )
	{
		setWoundGlowEffect( Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.5f), Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.0f) );
	}

	/*PreScabFrame::~PreScabFrame()
	{
	}*/


	MultiRTTSampler::SampledTexture PreScabFrame::renderFrame()
	{
		clock_t tStart = clock();
		std::cout << "starting rendering PreScabFrame" << std::endl;

		Ogre::Texture *woundHeightTex         = Renderer::getManualTexture("preScabFrameHeightTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		Ogre::Texture *woundHeightMaskOrigTex = Renderer::getManualTexture("preScabFrameHeightMaskTexOriginal", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		Ogre::Texture *woundHeightMaskTex     = Renderer::getManualTexture("preScabFrameHeightMaskTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		Ogre::Texture *specleNoiseTex         = Renderer::getManualTexture("preScabFrameNoiseSpotTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		Ogre::Texture *lowerNoiseTex          = Renderer::getManualTexture("preScabFrameLowerNoiseTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

		// only render noise if something has been drawn
		if ( SharedResources::patchEdgeListPtr != 0 && SharedResources::patchEdgeListPtr->size() >= 1 )
		{
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(woundHeightMaskOrigTex), 7, 1.0f, 0.44f, 0.02f, noiseSeed );
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(specleNoiseTex), 5, 0.48f, 0.16f, 0.15f, noiseSeed );
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(lowerNoiseTex), 5, 0.85f, 0.54f, 0.21f, noiseSeed );

			// fade to black
			Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingletonPtr()->getByName( woundHeightMaskTex->getName() );
			woundHeightMaskOrigTex->copyToTexture( texPtr );
			texPtr.setNull();

			GradientEdgeRenderer::fadeTo( dynamic_cast<Ogre::D3D11Texture*>(woundHeightMaskTex),
											dynamic_cast<Ogre::D3D11Texture*>(SharedResources::getTexture(SharedResources::WoundShapeUpperSkin)),
											SharedResources::patchEdgeListPtr, GradientEdgeRenderer::Side_Inner, Ogre::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 10U );
		}
		woundHeightTex = lowerNoiseTex;

		// setup albedo renderpass
		Ogre::Pass *renderPass            = Ogre::MaterialManager::getSingleton().getByName( albedoMatName )->getTechnique(0)->getPass(0);
		Ogre::Texture *woundShapeUpperTex = SharedResources::getTexture( SharedResources::WoundShapeUpperSkin );
		Ogre::TextureUnitState *texState  = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}
		Ogre::Texture *woundShapeLowerTex = SharedResources::getTexture( SharedResources::WoundShapeLowerSkin );
		texState = renderPass->getTextureUnitState( woundShapeLowerTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeLowerTex->getName() );
			texState->setName( woundShapeLowerTex->getName() );
		}
		texState = renderPass->getTextureUnitState( specleNoiseTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( specleNoiseTex->getName() );
			texState->setName( specleNoiseTex->getName() );
		}
		texState = renderPass->getTextureUnitState( lowerNoiseTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( lowerNoiseTex->getName() );
			texState->setName( lowerNoiseTex->getName() );
		}
		Ogre::Texture *upperSkinClrTex = SharedResources::getTexture(SharedResources::UpperSkinColor);
		texState = renderPass->getTextureUnitState( upperSkinClrTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( upperSkinClrTex->getName() );
			texState->setName( upperSkinClrTex->getName() );
		}
		Ogre::Texture *lowerSkinClrTex = SharedResources::getTexture(SharedResources::LowerSkinColor);
		texState = renderPass->getTextureUnitState( lowerSkinClrTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( lowerSkinClrTex->getName() );
			texState->setName( lowerSkinClrTex->getName() );
		}


		// setup normalmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( normalMatName )->getTechnique(0)->getPass(0);
		texState   = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}

		// setup reflectionmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( reflectionMatName )->getTechnique(0)->getPass(0);
		texState = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}

		// setup heightmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( heightMatName )->getTechnique(0)->getPass(0);
		const std::string WOUND_HEIGHT_TEXSTATE_NAME = "preScabFrameHeightTexState";
		texState = renderPass->getTextureUnitState( WOUND_HEIGHT_TEXSTATE_NAME );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState();
			texState->setName( WOUND_HEIGHT_TEXSTATE_NAME );
		}
		texState->setTextureName( woundHeightTex->getName() );			

		const std::string WOUND_HEIGHT_MASK_TEXSTATE_NAME = "preScabFrameHeightMaskTexState";
		texState = renderPass->getTextureUnitState( WOUND_HEIGHT_MASK_TEXSTATE_NAME );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState();
			texState->setName( WOUND_HEIGHT_MASK_TEXSTATE_NAME );
		}
		texState->setTextureName( woundHeightMaskTex->getName() );

		// render textures
		renderTexture( TextureType::Albedo );
		renderTexture( TextureType::Normal );
		renderTexture( TextureType::Reflection );
		renderTexture( TextureType::Height );


		SampledTexture output;
		output.albedo     = albedoTex.dataTex;
		output.normal     = normalTex.dataTex;
		output.reflection = reflectionTex.dataTex;
		output.height     = heightTex.dataTex;

		double time = double( clock() - tStart ) / (double)CLOCKS_PER_SEC;
		std::cout << "done in: " << time << " seconds" << std::endl;

		return output;
	}
}
