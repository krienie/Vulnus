
#include <iostream>

#include <OgreCamera.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>

#include "Utils.h"
#include "NoiseRendererGPU.h"
#include "SharedResources.h"
#include "Frames/VasoconstrictionFrame.h"
#include "Frames/VasodilationFrame.h"
#include "Frames/PreScabFrame.h"
#include "Frames/ScabFrame.h"
#include "Frames/MaturationFrame.h"
#include "WoundErosionFilter.h"
#include "WoundTimeline.h"

namespace Vulnus
{

	WoundTimeline::WoundTimeline()
		: MultiRTTSampler(), isInit(false),
		dxDevice(0), dxDevContext(0), uniqueFrames(), keyFrames(), timelineFrames()
	{
	}

	WoundTimeline::~WoundTimeline()
	{
		safeRelease(&dxDevContext);

		std::map<Keyframe*, int>::iterator it;
		for ( it = uniqueFrames.begin(); it != uniqueFrames.end(); ++it )
			delete it->first;
	}


	void WoundTimeline::initialize( _In_ ID3D11Device *dxDev, _In_ Renderer *ren, _In_ const std::vector<Utils::HullPoint> *patchEdgeListPtr )
	{
		dxDevice = dxDev;
		dxDevice->GetImmediateContext( &dxDevContext );

		Ogre::SceneManager *sceneMgr = Ogre::Root::getSingletonPtr()->createSceneManager("DefaultSceneManager");
		Ogre::Entity *timelinePlane  = Renderer::loadModel( "WoundTimelinePlane", "viewportPlane.mesh", sceneMgr );

		Ogre::Camera *sceneCam = sceneMgr->createCamera("TimelineCam");
		sceneCam->setPosition(Ogre::Vector3(20,20,20));
		sceneCam->lookAt( Ogre::Vector3(0,0,0) );
		sceneCam->setNearClipDistance(5);
		sceneCam->setFarClipDistance(0);

		int width  = ren->getMainRenderWidget()->width();
		int height = ren->getMainRenderWidget()->height();
		sceneCam->setAspectRatio( Ogre::Real(width) / Ogre::Real(height) );

		// initialize Render-To-Texture Sampler
		init( sceneCam, timelinePlane, "TextureBlend" );

		// fill timeline
		Keyframe *vasoconstrictFrame = new VasoconstrictionFrame( sceneCam, timelinePlane );
		Keyframe *vasodilationFrame  = new VasodilationFrame( sceneCam, timelinePlane );
		Keyframe *preScabFrame       = new PreScabFrame( sceneCam, timelinePlane );
		Keyframe *scabFrame          = new ScabFrame( sceneCam, timelinePlane );

		Ogre::D3D11RenderSystem *renderSys = dynamic_cast<Ogre::D3D11RenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem());
		Keyframe *maturationFrame          = new MaturationFrame( sceneCam, timelinePlane, renderSys->_getDevice().get() );

		addKeyframe( 0.1155, vasoconstrictFrame );
		addKeyframe( 0.1658, vasodilationFrame );
		addKeyframe( 0.2562, vasodilationFrame );
		addKeyframe( 0.3015, preScabFrame );
		addKeyframe( 0.3919, preScabFrame );
		addKeyframe( 0.6934, scabFrame );
		addKeyframe( 0.69341, maturationFrame );


		SharedResources::addTexture( SharedResources::UpperSkinColor, "Hand_TXTR.tif");
		SharedResources::addTexture( SharedResources::LowerSkinColor, "Hand_TXTR_LowerSkin.tif");
		SharedResources::addTexture( SharedResources::WoundShapeUpperSkin, "woundShapeUpperSkinTexture" );
		SharedResources::addTexture( SharedResources::WoundShapeLowerSkin, "woundShapeLowerSkinTexture" );

		Ogre::Texture* woundInternalTex = Renderer::getManualTexture("woundColorInsideNoise", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(woundInternalTex), 6, 0.5f );
		SharedResources::addTexture( SharedResources::WoundColor, woundInternalTex->getName() );

		SharedResources::patchEdgeListPtr = patchEdgeListPtr;

		updateFrames();

		isInit = true;
	}


	void WoundTimeline::updateFrames()
	{
		std::list<FramePosition>::iterator it;
		for ( it = keyFrames.begin(); it != keyFrames.end(); ++it )
			it->frame->renderFrame();
	}

	MultiRTTSampler::SampledTexture WoundTimeline::sample( double position )
	{
		if ( !isInit )
		{
			std::cout << "(WoundTimeline::sample) Error: WoundTimelineWidget is not initialized." << std::endl;
			return SampledTexture();
		}

		if ( keyFrames.empty() )
		{
			std::cout << "(WoundTimeline::sample) Error: Unable to sample. Need at least one keyframe." << std::endl;
			return SampledTexture();
		}

		Ogre::Material *sampleMat = getSampleMaterialPtr();
		if ( !sampleMat )
			return SampledTexture();


		float samplePos = Ogre::Math::Clamp<double>(position, 0.0f, 1.0f);

		// find the two keyframes position is between and calculate the lerpValue using the frame positions
		std::forward_list<FramePosition>::iterator it = timelineFrames.begin();
		std::forward_list<FramePosition>::iterator lastElemIt = timelineFrames.before_begin();
		std::forward_list<FramePosition>::iterator leftKeyframe  = it;
		std::forward_list<FramePosition>::iterator rightKeyframe = it;
		for ( it; it != timelineFrames.end(); ++it, ++lastElemIt )
		{
			if ( it->pos <= samplePos )
				leftKeyframe = it;

			if ( leftKeyframe->pos <= samplePos && it->pos >= samplePos )
			{
				rightKeyframe = it;
				break;
			}
		}
		if ( rightKeyframe == timelineFrames.end() )
			rightKeyframe = lastElemIt;

		if ( leftKeyframe != rightKeyframe )
		{
			float temp = rightKeyframe->pos - leftKeyframe->pos;
			temp = temp == 0.0f ? 1.0f : temp;
			samplePos = (samplePos - leftKeyframe->pos) / temp;
		} else
		{
			samplePos = leftKeyframe->pos;
		}



		// set lerpValue constant parameter
		Ogre::Pass *samplePass = sampleMat->getTechnique(0)->getPass(0);
		Ogre::GpuProgramParametersSharedPtr fragParamsPtr = samplePass->getFragmentProgramParameters();
		Ogre::GpuProgramParameters *fragParams = fragParamsPtr.get();
		fragParamsPtr.setNull();
		fragParams->setNamedConstant("lerpValue", samplePos );
		

		// get material texture states
		Ogre::TextureUnitState *texState1 = samplePass->getTextureUnitState("texture1");
		if ( !texState1 )
		{
			texState1 = samplePass->createTextureUnitState();
			texState1->setName("texture1");
		}
		Ogre::TextureUnitState *texState2 = samplePass->getTextureUnitState("texture2");
		if ( !texState2 )
		{
			texState2 = samplePass->createTextureUnitState();
			texState2->setName("texture2");
		}

		//sample left and right keyframe
		SampledTexture leftSample  = leftKeyframe->frame->sample( samplePos );
		SampledTexture rightSample = leftSample;//rightKeyframe->frame->sample( samplePos );

		if ( leftKeyframe->frame != rightKeyframe->frame )
			rightSample = rightKeyframe->frame->sample( samplePos );

		// update albedo texture
		texState1->setTextureName( leftSample.albedo->getName() );
		texState2->setTextureName( rightSample.albedo->getName() );
		updateSampleTexture(MultiRTTSampler::Albedo);
		// update normal texture
		texState1->setTextureName( leftSample.normal->getName() );
		texState2->setTextureName( rightSample.normal->getName() );
		updateSampleTexture(MultiRTTSampler::Normal);
		// update reflection texture
		texState1->setTextureName( leftSample.reflection->getName() );
		texState2->setTextureName( rightSample.reflection->getName() );
		updateSampleTexture(MultiRTTSampler::Reflection);
		// update height texture
		texState1->setTextureName( leftSample.height->getName() );
		texState2->setTextureName( rightSample.height->getName() );
		updateSampleTexture(MultiRTTSampler::Height);

		// get results
		RTTTexture albedoRes  = getSampledTexture(MultiRTTSampler::Albedo);
		RTTTexture normalRes  = getSampledTexture(MultiRTTSampler::Normal);
		RTTTexture reflectRes = getSampledTexture(MultiRTTSampler::Reflection);
		RTTTexture heightRes  = getSampledTexture(MultiRTTSampler::Height);

		SampledTexture outputTextures;
		outputTextures.albedo     = albedoRes.dataTex;
		outputTextures.normal     = normalRes.dataTex;
		outputTextures.reflection = reflectRes.dataTex;
		outputTextures.height     = heightRes.dataTex;

		return outputTextures;
	}


	void WoundTimeline::addKeyframe( double position, _In_ Keyframe *kf )
	{
		// check if the added keyframe is already known
		std::map<Keyframe*, int>::iterator uniqueIt = uniqueFrames.find(kf);
		if ( uniqueIt == uniqueFrames.end() )
			uniqueFrames.insert( std::pair<Keyframe*, int>(kf, 0) );


		double newFramePos = Ogre::Math::Clamp<double>(position, 0.0, 1.0);

		keyFrames.push_front( FramePosition(newFramePos, kf) );
		keyFrames.sort( sortFrames );

		timelineFrames = std::forward_list<FramePosition>();
		std::list<FramePosition>::iterator firstElemIt = keyFrames.begin();
		std::list<FramePosition>::iterator lastElemIt  = keyFrames.end();
		lastElemIt--;

		// add first and last frame timeline
		if ( firstElemIt->pos > 0.0 )
			timelineFrames.push_front( FramePosition(0.0, firstElemIt->frame) );
		if ( lastElemIt->pos < 1.0 )
			timelineFrames.push_front( FramePosition(1.0f, lastElemIt->frame) );

		// add existing frames to timeline
		std::list<FramePosition>::const_iterator it;
		for ( it = keyFrames.cbegin(); it != keyFrames.cend(); ++it )
			timelineFrames.push_front( *it );

		timelineFrames.sort( sortFrames );
	}


	bool sortFrames( const Vulnus::FramePosition f1, const Vulnus::FramePosition f2 )
	{
		return f1.pos < f2.pos;
	}
}
