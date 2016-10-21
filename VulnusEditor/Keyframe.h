
#pragma once

#include <d3d11.h>

#include "MultiRTTSampler.h"

namespace Vulnus
{
	class Keyframe : public MultiRTTSampler
	{
		public:
			Keyframe( const std::string name, const std::string &albedoMat, const std::string &normalMat,
						const std::string &reflectMat, const std::string &heightMat,
						_In_ Ogre::Camera *camera, _In_ Ogre::Entity *model );
			virtual ~Keyframe();

			//inline bool operator< ( const Keyframe &rhs ){ return pos < rhs.pos; }

			virtual SampledTexture renderFrame() = 0;
			virtual SampledTexture sample( double position );
		protected:
			void setWoundGlowEffect( const Ogre::Vector4 &glowFrom, const Ogre::Vector4 &glowTo );
			void renderTexture( const TextureType &texType );
			void applyWoundGlowEffect();

			RTTTexture albedoTex;
			RTTTexture normalTex;
			RTTTexture reflectionTex;
			RTTTexture heightTex;
			std::string albedoMatName;
			std::string normalMatName;
			std::string reflectionMatName;
			std::string heightMatName;

		private:
			Ogre::Entity *renderModel;
			std::string frameName;

			ID3D11Device *dxDevice;
			ID3D11DeviceContext *dxDevContext;

			// prerender resources
			RTTTexture woundGlowBlendTex;
			RTTTexture woundGlowTex;
			std::string woundGlowBlendMat;
			Ogre::Vector4 woundGlowFrom;
			Ogre::Vector4 woundGlowTo;
	};
}
