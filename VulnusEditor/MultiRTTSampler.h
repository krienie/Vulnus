
#pragma once

#include <OgreTexture.h>
#include <map>


namespace Vulnus
{
	class MultiRTTSampler
	{
		public:
			enum TextureType
			{
				Albedo,
				Normal,
				Reflection,
				Height
			};

			struct RTTTexture
			{
				Ogre::Texture *dataTex;
				Ogre::RenderTexture *renderTex;
			};

			struct SampledTexture
			{
				SampledTexture() : albedo(0), normal(0), reflection(0), height(0) {}

				Ogre::Texture *albedo;
				Ogre::Texture *normal;
				Ogre::Texture *reflection;
				Ogre::Texture *height;
			};


			MultiRTTSampler();
			MultiRTTSampler( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model, const std::string &matName );
			virtual ~MultiRTTSampler();

			virtual SampledTexture sample( double position ) = 0;

		protected:
			void init( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model, const std::string &matName );
			void createRTTResources( const std::string &texName, _Inout_ Ogre::Texture **texture, _Inout_ Ogre::RenderTexture **renderTexture );
			RTTTexture createRTTResources( const std::string &texName );
			void updateSampleTextures();
			void updateSampleTexture( const TextureType &texType, const std::string customSampleMat = "" );
			Ogre::Material* getSampleMaterialPtr() const;
			RTTTexture getSampledTexture( const TextureType &texType ) const;

			Ogre::Camera *sampleCam;
			std::string sampleMatName;

		private:
			static int rttSampleCounter;

			bool isInit;
			Ogre::Entity *sampleModel;
			std::map<TextureType, RTTTexture> sampleTextures;
	};
}
