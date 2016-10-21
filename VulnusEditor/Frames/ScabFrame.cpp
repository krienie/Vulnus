
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>
#include <OgreTechnique.h>

#include "Renderer.h"
#include "GradientEdgeRenderer.h"
#include "NoiseRendererGPU.h"
#include "SharedResources.h"
#include "ScabFrame.h"

namespace Vulnus
{
	ScabFrame::ScabFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model )
		: Keyframe("ScabFrame", "scabFrameAlbedo", "scabFrameNormal",
						"scabFrameReflection", "scabFrameHeight", camera, model),
			noiseSeed( NoiseRendererGPU::getNewSeed() )
	{
		setWoundGlowEffect( Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.5f), Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.0f) );
	}

	/*ScabFrame::~ScabFrame()
	{
	}*/


	MultiRTTSampler::SampledTexture ScabFrame::renderFrame()
	{
		clock_t tStart = clock();
		std::cout << "starting rendering ScabFrame" << std::endl;

		Ogre::Texture *woundHeightTex         = Renderer::getManualTexture( "scabFrameHeightTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *woundHeightMaskOrigTex = Renderer::getManualTexture( "scabFrameHeightMaskTexOriginal", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *woundHeightMaskTex     = Renderer::getManualTexture( "scabFrameHeightMaskTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *woundEdgeColorTex      = Renderer::getManualTexture( "scabFrameWoundEdgeColorTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		Ogre::Texture *woundEdgeColorMaskTex  = Renderer::getManualTexture( "scabFrameWoundEdgeColorMaskTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );

		Ogre::Texture *reflectanceTex = Renderer::getManualTexture( "scabFrameReflectanceTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE );
		if ( SharedResources::patchEdgeListPtr != 0 && SharedResources::patchEdgeListPtr->size() >= 1 )
		{
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(woundHeightMaskOrigTex), 7, 1.0f, 0.44f, 0.02f, noiseSeed, 1, 0.45f );
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(woundHeightTex), 5, 0.54f, 0.30f, 0.69f, noiseSeed );
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(reflectanceTex), 10, 1.00f, 0.25f, 0.95f, noiseSeed );
			NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(woundEdgeColorMaskTex), 5, 0.33f, 0.60f, 0.59f, noiseSeed );

			// fade to black
			Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingletonPtr()->getByName( woundHeightMaskTex->getName() );
			woundHeightMaskOrigTex->copyToTexture( texPtr );
			texPtr.setNull();

			// create color map gradient
			GradientEdgeRenderer::fadeTo( dynamic_cast<Ogre::D3D11Texture*>(woundEdgeColorTex),
												dynamic_cast<Ogre::D3D11Texture*>(SharedResources::getTexture(SharedResources::WoundShapeUpperSkin)),
												SharedResources::patchEdgeListPtr, GradientEdgeRenderer::Side_Inner,
												//Ogre::Vector4(1.0000f, 0.2109f, 0.1679f, 1.0f), Ogre::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 100U );
												Ogre::Vector4(0.5000f, 0.0429f, 0.0351f, 1.0f), Ogre::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 100U );
			

			// create heightmap gradient
			GradientEdgeRenderer::fadeTo( dynamic_cast<Ogre::D3D11Texture*>(woundHeightMaskTex),
												dynamic_cast<Ogre::D3D11Texture*>(SharedResources::getTexture(SharedResources::WoundShapeUpperSkin)),
												SharedResources::patchEdgeListPtr, GradientEdgeRenderer::Side_Inner, Ogre::Vector4(0.0f, 0.0f, 0.0f, 1.0f), 10U );
		}
		woundHeightMaskOrigTex = SharedResources::getTexture( SharedResources::WoundShapeUpperSkin );

		Ogre::GpuProgramParameters *albedoPixelParams = Utils::getFragmentParameters( albedoMatName );
		albedoPixelParams->setNamedConstant( "darkScabClr", Ogre::Vector4(0.3906f, 0.1171f, 0.1054f, 1.0f) );
		albedoPixelParams->setNamedConstant( "lightScabClr", Ogre::Vector4(0.7812f, 0.4804f, 0.2773f, 1.0f) );
		//albedoPixelParams->setNamedConstant( "lightScabClr", Ogre::Vector4(0.6250f, 0.3828f, 0.2226f, 1.0f) );

		// setup albedo renderpass
		Ogre::Pass *renderPass = Ogre::MaterialManager::getSingleton().getByName( albedoMatName )->getTechnique(0)->getPass(0);
		Ogre::Texture *woundShapeUpperTex = SharedResources::getTexture( SharedResources::WoundShapeUpperSkin );
		Ogre::TextureUnitState *texState  = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}
		texState = renderPass->getTextureUnitState( woundEdgeColorTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundEdgeColorTex->getName() );
			texState->setName( woundEdgeColorTex->getName() );
		}
		texState = renderPass->getTextureUnitState( woundEdgeColorMaskTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundEdgeColorMaskTex->getName() );
			texState->setName( woundEdgeColorMaskTex->getName() );
		}
		texState = renderPass->getTextureUnitState( woundHeightTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundHeightTex->getName() );
			texState->setName( woundHeightTex->getName() );
		}


		// setup reflectionmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( reflectionMatName )->getTechnique(0)->getPass(0);
		texState = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}

		texState = renderPass->getTextureUnitState( reflectanceTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( reflectanceTex->getName() );
			texState->setName( reflectanceTex->getName() );
		}

		// setup normalmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( normalMatName )->getTechnique(0)->getPass(0);
		texState = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}

		// setup heightmap renderpass
		renderPass = Ogre::MaterialManager::getSingleton().getByName( heightMatName )->getTechnique(0)->getPass(0);
		const std::string WOUND_HEIGHT_TEXSTATE_NAME = "scabFrameHeightTexState";
		texState = renderPass->getTextureUnitState( WOUND_HEIGHT_TEXSTATE_NAME );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState();
			texState->setName( WOUND_HEIGHT_TEXSTATE_NAME );
		}
		texState->setTextureName( woundHeightTex->getName() );			

		const std::string WOUND_HEIGHT_MASK_TEXSTATE_NAME = "scabFrameHeightMaskTexState";
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


	MultiRTTSampler::SampledTexture ScabFrame::sample( double position )
	{
		float samplePos = Ogre::Math::Clamp<float>( float(position), 0.0f, 1.0f);
		float dispScale = std::powf( 1000.0f, static_cast<float>(position) - 1.40f ) + 0.1f;		// 1000^(x - 1.30) + 0.1

		Ogre::GpuProgramParameters *gpuProgParams = Utils::getDomainParameters("maleHand");
		gpuProgParams->setNamedConstant( "displacementScale", dispScale );

		SampledTexture output;
		output.albedo     = albedoTex.dataTex;
		output.normal     = normalTex.dataTex;
		output.reflection = reflectionTex.dataTex;
		output.height     = heightTex.dataTex;

		return output;
	}
}
