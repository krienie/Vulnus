
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

#include "SharedResources.h"
#include "VasoconstrictionFrame.h"

namespace Vulnus
{
	VasoconstrictionFrame::VasoconstrictionFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model )
		: Keyframe("VasoconstrictionFrame", "vasoconstrictionFrameAlbedo", "vasoconstrictionFrameNormal",
						"vasoconstrictionFrameReflection", "vasoconstrictionFrameHeight", camera, model)
	{
		//setWoundGlowEffect( Ogre::Vector4(0.0f, 0.0f, 0.0f, 0.0f), Ogre::Vector4(0.0f, 0.0f, 0.0f, 0.0f) );
	}

	/*VasoconstrictionFrame::~VasoconstrictionFrame()
	{
	}*/


	MultiRTTSampler::SampledTexture VasoconstrictionFrame::renderFrame()
	{
		clock_t tStart = clock();
		std::cout << "starting rendering VasoconstrictionFrame" << std::endl;

		// setup albedo renderpass
		Ogre::Pass *renderPass            = Ogre::MaterialManager::getSingleton().getByName( albedoMatName )->getTechnique(0)->getPass(0);
		Ogre::Texture *woundShapeUpperTex = SharedResources::getTexture( SharedResources::WoundShapeUpperSkin );
		Ogre::TextureUnitState *texState  = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}
		Ogre::Texture *skinClrTex = SharedResources::getTexture(SharedResources::LowerSkinColor);
		texState = renderPass->getTextureUnitState( skinClrTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( skinClrTex->getName() );
			texState->setName( skinClrTex->getName() );
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
