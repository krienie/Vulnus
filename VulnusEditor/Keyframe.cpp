
#include <OgreD3D11RenderSystem.h>
#include <OgreD3D11Texture.h>
#include <OgreEntity.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreRenderTexture.h>

#include "GradientEdgeRenderer.h"
#include "SharedResources.h"
#include "Keyframe.h"

namespace Vulnus
{
	Keyframe::Keyframe( const std::string name, const std::string &albedoMat, const std::string &normalMat,
						const std::string &reflectMat, const std::string &heightMat,
						_In_ Ogre::Camera *camera, _In_ Ogre::Entity *model )
		: MultiRTTSampler(camera, model, "TextureBlend"), albedoTex(), normalTex(), reflectionTex(),
			heightTex(), albedoMatName(albedoMat), normalMatName(normalMat), reflectionMatName(reflectMat),
			heightMatName(heightMat), frameName(name), renderModel(model), dxDevice(0), dxDevContext(0),
			woundGlowBlendTex(), woundGlowTex(), woundGlowBlendMat("WoundGlowBlend"), woundGlowFrom(0.0f), woundGlowTo(0.0f)
	{
		std::stringstream ssAlbedo;
		ssAlbedo << frameName << "Albedo";
		albedoTex = createRTTResources( ssAlbedo.str() );

		std::stringstream ssNormal;
		ssNormal << frameName << "Normal";
		normalTex = createRTTResources( ssNormal.str() );

		std::stringstream ssReflection;
		ssReflection << frameName << "Reflection";
		reflectionTex = createRTTResources( ssReflection.str() );

		std::stringstream ssHeight;
		ssHeight << frameName << "Height";
		heightTex = createRTTResources( ssHeight.str() );

		std::stringstream ssPreRenderBlend;
		ssPreRenderBlend << frameName << "WoundGlowBlend";
		woundGlowBlendTex = createRTTResources( ssPreRenderBlend.str() );

		std::stringstream ssPreRender;
		ssPreRender << frameName << "WoundGlow";
		woundGlowTex = createRTTResources( ssPreRender.str() );

		// get DirectX Device
		Ogre::D3D11RenderSystem *dxRenderSys = dynamic_cast<Ogre::D3D11RenderSystem*>( Ogre::Root::getSingletonPtr()->getRenderSystem() );
		dxDevice = dxRenderSys->_getDevice().get();
		dxDevice->GetImmediateContext( &dxDevContext );
	}

	Keyframe::~Keyframe()
	{
		if ( dxDevContext )
			dxDevContext->Release();
	}


	void Keyframe::setWoundGlowEffect( const Ogre::Vector4 &glowFrom, const Ogre::Vector4 &glowTo )
	{
		woundGlowFrom = glowFrom;
		woundGlowTo   = glowTo;
	}


	void Keyframe::renderTexture( const TextureType &texType )
	{
		if ( texType == TextureType::Albedo )
		{
			Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( albedoMatName );
			renderModel->setMaterial( matPtr );
			matPtr.setNull();
			albedoTex.renderTex->update();

			applyWoundGlowEffect();
		} else if ( texType == TextureType::Normal )
		{
			Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( normalMatName );
			renderModel->setMaterial( matPtr );
			matPtr.setNull();

			normalTex.renderTex->update();
		} else if ( texType == TextureType::Reflection )
		{
			Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( reflectionMatName );
			renderModel->setMaterial( matPtr );
			matPtr.setNull();

			reflectionTex.renderTex->update();
		} else if ( texType == TextureType::Height )
		{
			Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( heightMatName );
			renderModel->setMaterial( matPtr );
			matPtr.setNull();

			heightTex.renderTex->update();
		}
	}

	MultiRTTSampler::SampledTexture Keyframe::sample( double position )
	{
		SampledTexture outputTextures;
		outputTextures.albedo     = albedoTex.dataTex;
		outputTextures.normal     = normalTex.dataTex;
		outputTextures.reflection = reflectionTex.dataTex;
		outputTextures.height     = heightTex.dataTex;

		return outputTextures;
	}


	void Keyframe::applyWoundGlowEffect()
	{
		// render preRender texture
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( "OutputTexture" );

		Ogre::Pass *renderPass            = matPtr->getTechnique(0)->getPass(0);
		Ogre::Texture *upperSkinTex = SharedResources::getTexture( SharedResources::UpperSkinColor );
		Ogre::TextureUnitState *texState  = renderPass->getTextureUnitState( upperSkinTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( upperSkinTex->getName() );
			texState->setName( upperSkinTex->getName() );
		}

		renderModel->setMaterial( matPtr );
		matPtr.setNull();
		woundGlowTex.renderTex->update();

		

		if ( SharedResources::patchEdgeListPtr == 0 || (SharedResources::patchEdgeListPtr != 0 && SharedResources::patchEdgeListPtr->size() == 0) )
			return;

		//std::cout << "Applying WoundGlowEffect!" << std::endl;
		/*GradientEdgeRenderer::fadeTo( dynamic_cast<Ogre::D3D11Texture*>(woundGlowTex.dataTex),
												dynamic_cast<Ogre::D3D11Texture*>(SharedResources::getTexture(SharedResources::WoundShapeUpperSkin)),
												SharedResources::patchEdgeListPtr, GradientEdgeRenderer::Side_Outer, Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.5f),
												Ogre::Vector4(0.7812f, 0.0f, 0.0f, 0.0f) );*/
		GradientEdgeRenderer::fadeTo( dynamic_cast<Ogre::D3D11Texture*>(woundGlowTex.dataTex),
												dynamic_cast<Ogre::D3D11Texture*>(SharedResources::getTexture(SharedResources::WoundShapeUpperSkin)),
												SharedResources::patchEdgeListPtr, GradientEdgeRenderer::Side_Outer, woundGlowFrom, woundGlowTo );

		// combine preRender texture with the keyframe texture
		matPtr = Ogre::MaterialManager::getSingleton().getByName( woundGlowBlendMat );
		renderModel->setMaterial( matPtr );
		

		// set textures
		/*Ogre::Pass **/renderPass           = matPtr->getTechnique(0)->getPass(0);
		/*Ogre::TextureUnitState **/texState = renderPass->getTextureUnitState( "WoundGlowKeyFrameTexState" );		
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState();
			texState->setName( "WoundGlowKeyFrameTexState" );
		}
		texState->setTextureName( albedoTex.dataTex->getName() );
		texState = renderPass->getTextureUnitState( "WoundGlowWoundGlowTexState" );		
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState();
			texState->setName( "WoundGlowWoundGlowTexState" );
		}
		texState->setTextureName( woundGlowTex.dataTex->getName() );
		Ogre::Texture *woundShapeUpperTex = SharedResources::getTexture( SharedResources::WoundShapeUpperSkin );
		texState = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}

		woundGlowBlendTex.renderTex->update();

		// copy result to keyframe texture
		Ogre::D3D11Texture *dxKeyframeTex       = dynamic_cast<Ogre::D3D11Texture*>(albedoTex.dataTex);
		Ogre::D3D11Texture *dxWoundGlowBlendTex = dynamic_cast<Ogre::D3D11Texture*>(woundGlowBlendTex.dataTex);
		dxDevContext->CopyResource( dxKeyframeTex->GetTex2D(), dxWoundGlowBlendTex->GetTex2D() );

		matPtr.setNull();
	}
}
