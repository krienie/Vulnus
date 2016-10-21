
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

#include "Renderer.h"
#include "GradientEdgeRenderer.h"
#include "NoiseRendererGPU.h"
#include "SharedResources.h"
#include "VasodilationFrame.h"

namespace Vulnus
{
	VasodilationFrame::VasodilationFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model )
		: Keyframe("VasodilationFrame", "vasodilationFrameAlbedo", "vasodilationFrameNormal",
						"vasodilationFrameReflection", "vasodilationFrameHeight", camera, model),
			noiseSeed( NoiseRendererGPU::getNewSeed() )
	{
		setWoundGlowEffect( Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.8f), Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.0f) );
	}

	/*VasodilationFrame::~VasodilationFrame()
	{
	}*/


	MultiRTTSampler::SampledTexture VasodilationFrame::renderFrame()
	{
		clock_t tStart = clock();
		std::cout << "starting rendering VasodilationFrame" << std::endl;

		Ogre::Texture *patchTex           = Renderer::getManualTexture( "VasoDilateFramePatchTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *reflectNoiseTex    = Renderer::getManualTexture( "VasoDilateFrameReflectNoiseTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *reflectNoiseMapTex = Renderer::getManualTexture( "VasoDilateFrameReflectNoiseMapTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );

		// only process if something has been drawn
		if ( SharedResources::getTexture(SharedResources::WoundShapeUpperSkin) != 0
				&& SharedResources::patchEdgeListPtr != 0 && SharedResources::patchEdgeListPtr->size() >= 1 )
		{
			/*clock_t tStart = clock();
			GradientEdgeRenderer::fadeToAlpha( dynamic_cast<Ogre::D3D11Texture*>(patchTex),
												dynamic_cast<Ogre::D3D11Texture*>(SharedResources::getTexture(SharedResources::WoundShapeUpperSkin)),
												SharedResources::patchEdgeListPtr, GradientEdgeRenderer::Side_Outer, 0.0f );
			double time = double( clock() - tStart ) / (double)CLOCKS_PER_SEC;
			std::cout << "done in: " << time << " seconds" << std::endl;*/

			// render reflectionmap texture
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(reflectNoiseTex), 5, 0.09f, 0.50f, 0.43f, noiseSeed, 64 );
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(reflectNoiseMapTex), 7, 0.31f, 0.52f, 0.57f, noiseSeed );
		}

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
		texState = renderPass->getTextureUnitState( patchTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( patchTex->getName() );
			texState->setName( patchTex->getName() );
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
		texState  = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
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
		texState = renderPass->getTextureUnitState( reflectNoiseTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( reflectNoiseTex->getName() );
			texState->setName( reflectNoiseTex->getName() );
		}
		texState = renderPass->getTextureUnitState( reflectNoiseMapTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( reflectNoiseMapTex->getName() );
			texState->setName( reflectNoiseMapTex->getName() );
		}

		// setup heightmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( heightMatName )->getTechnique(0)->getPass(0);
		texState   = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}


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
