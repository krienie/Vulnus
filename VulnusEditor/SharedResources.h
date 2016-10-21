
#pragma once

#include <OgreTexture.h>

#include "Utils.h"

namespace Vulnus
{
	class SharedResources
	{
		public:
			enum SharedTextureType
			{
				UpperSkinColor,
				LowerSkinColor,
				WoundShapeUpperSkin,
				WoundShapeLowerSkin,
				WoundColor
			};

			static bool addTexture( SharedTextureType type, const std::string &textureName );
			static Ogre::Texture* getTexture( SharedTextureType type );

			static const std::vector<Utils::HullPoint>* patchEdgeListPtr;

		private:
			SharedResources();
			~SharedResources();
			SharedResources( const SharedResources& );
			SharedResources& operator=( const SharedResources& );

			typedef std::map<SharedResources::SharedTextureType, Ogre::Texture*>::iterator SharedTexIterator;
			static std::map<SharedTextureType, Ogre::Texture*> sharedTextures;
	};
}