
#include <OgreEntity.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreRenderTexture.h>
#include <OgreTextureManager.h>
#include <OgreViewport.h>

#include "Renderer.h"
#include "MultiRTTSampler.h"

namespace Vulnus
{
	int MultiRTTSampler::rttSampleCounter = 0;

	MultiRTTSampler::MultiRTTSampler()
		: sampleTextures(), sampleCam(0), sampleMatName(), isInit(false), sampleModel(0)
	{
	}

	MultiRTTSampler::MultiRTTSampler( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model, const std::string &matName )
		: sampleTextures(), sampleCam(0), sampleMatName(), isInit(false), sampleModel(0)
	{
		init( camera, model, matName );
	}

	MultiRTTSampler::~MultiRTTSampler()
	{
	}

	void MultiRTTSampler::init( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model, const std::string &matName )
	{
		if ( isInit )
			return;

		sampleCam     = camera;
		sampleMatName = matName;
		sampleModel   = model;

		isInit = true;

		MultiRTTSampler::rttSampleCounter++;
		std::stringstream ssAlbedo;
		ssAlbedo << "MultiRTTSampleTextureAlbedo" << MultiRTTSampler::rttSampleCounter;
		sampleTextures.insert( std::pair<TextureType, RTTTexture>(TextureType::Albedo, createRTTResources(ssAlbedo.str())) );

		std::stringstream ssNormal;
		ssNormal << "MultiRTTSampleTextureNormal" << MultiRTTSampler::rttSampleCounter;
		sampleTextures.insert( std::pair<TextureType, RTTTexture>(TextureType::Normal, createRTTResources(ssNormal.str())) );

		std::stringstream ssReflection;
		ssReflection << "MultiRTTSampleTextureReflection" << MultiRTTSampler::rttSampleCounter;
		sampleTextures.insert( std::pair<TextureType, RTTTexture>(TextureType::Reflection, createRTTResources(ssReflection.str())) );

		std::stringstream ssHeight;
		ssHeight << "MultiRTTSampleTextureHeight" << MultiRTTSampler::rttSampleCounter;
		sampleTextures.insert( std::pair<TextureType, RTTTexture>(TextureType::Height, createRTTResources(ssHeight.str())) );
	}

	void MultiRTTSampler::createRTTResources( const std::string &texName, _Inout_ Ogre::Texture **texture, _Inout_ Ogre::RenderTexture **renderTexture )
	{
		if ( !isInit )
		{
			std::cout << "(MultiRTTSampler::createRTTResources) Error: RTTSampler is not initialized." << std::endl;
			return;
		}

		if ( !texture || !renderTexture )
		{
			std::cout << "(MultiRTTSampler::createRTTResources) Error: Texture and/or RenderTexture pointer(s) not defined." << std::endl;
			return;
		}

		Ogre::TexturePtr rttTex = Ogre::TextureManager::getSingleton().createManual( texName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																					Ogre::TEX_TYPE_2D, Renderer::GLOBAL_TEX_WIDTH,
																					Renderer::GLOBAL_TEX_HEIGHT, 0, Ogre::PF_R8G8B8A8, Ogre::TU_RENDERTARGET );

		Ogre::HardwarePixelBufferSharedPtr pixBuff = rttTex->getBuffer();
		Ogre::Viewport *vp = pixBuff->getRenderTarget()->addViewport(sampleCam);
		vp->setClearEveryFrame(true);
		vp->setBackgroundColour( Renderer::VIEWPORT_CLEAR_COLOR );
		vp->setOverlaysEnabled(false);

		*texture       = rttTex.get();
		*renderTexture = pixBuff->getRenderTarget();

		rttTex.setNull();
		pixBuff.setNull();
	}

	MultiRTTSampler::RTTTexture MultiRTTSampler::createRTTResources( const std::string &texName )
	{
		RTTTexture sampledTexs;
		createRTTResources( texName, &sampledTexs.dataTex, &sampledTexs.renderTex );

		return sampledTexs;
	}


	void MultiRTTSampler::updateSampleTextures()
	{
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( sampleMatName );
		if ( matPtr.isNull() )
		{
			std::cout << "(MultiRTTSampler::updateSampleTextures) Warning: Material " << sampleMatName << " was not found!" << std::endl;
			return;
		}
		sampleModel->setMaterial( matPtr );
		matPtr.setNull();

		std::map<TextureType, RTTTexture>::iterator it;
		for ( it = sampleTextures.begin(); it != sampleTextures.end(); ++it )
			it->second.renderTex->update();
	}

	void MultiRTTSampler::updateSampleTexture( const MultiRTTSampler::TextureType &texType, const std::string customSampleMat )
	{
		std::string curSampleMat = customSampleMat.length() == 0 ? sampleMatName : customSampleMat;

		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( curSampleMat );
		if ( matPtr.isNull() )
		{
			std::cout << "(MultiRTTSampler::updateSampleTexture) Warning: Material " << curSampleMat << " was not found!" << std::endl;
			return;
		}
		sampleModel->setMaterial( matPtr );
		matPtr.setNull();

		std::map<TextureType, RTTTexture>::const_iterator it = sampleTextures.find(texType);
		it->second.renderTex->update();
	}


	Ogre::Material* MultiRTTSampler::getSampleMaterialPtr() const
	{
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( sampleMatName );
		if ( matPtr.isNull() )
		{
			std::cout << "(MultiRTTSampler::getSampleMaterialPtr) Warning: Material " << sampleMatName << " was not found!" << std::endl;
			return NULL;
		}
		Ogre::Material *material = matPtr.get();
		matPtr.setNull();

		return material;
	}

	MultiRTTSampler::RTTTexture MultiRTTSampler::getSampledTexture( const MultiRTTSampler::TextureType &texType ) const
	{
		std::map<TextureType, RTTTexture>::const_iterator it = sampleTextures.find(texType);
		return it->second;
	}
}
