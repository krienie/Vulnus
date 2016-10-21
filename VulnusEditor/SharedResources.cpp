
#include <sstream>

#include <OgreHardwarePixelBuffer.h>
#include <OgreTextureManager.h>

#include "Renderer.h"
#include "SharedResources.h"

namespace Vulnus
{
	std::map<SharedResources::SharedTextureType, Ogre::Texture*> SharedResources::sharedTextures = std::map<SharedTextureType, Ogre::Texture*>();
	const std::vector<Utils::HullPoint>* SharedResources::patchEdgeListPtr = NULL;

	bool SharedResources::addTexture( SharedResources::SharedTextureType type, const std::string &textureName )
	{
		SharedTexIterator it = sharedTextures.find(type);
		if ( it != sharedTextures.end() )
			return false;

		Ogre::TexturePtr sharedTexPtr = Ogre::TextureManager::getSingletonPtr()->getByName( textureName );
		if ( sharedTexPtr.isNull() )
			sharedTexPtr = Ogre::TextureManager::getSingletonPtr()->load( textureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );

		Ogre::Texture *shareTexture = NULL;
		if ( sharedTexPtr.isNull() )
			shareTexture = Renderer::getManualTexture(textureName, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		else shareTexture = sharedTexPtr.get();

		sharedTexPtr.setNull();

		sharedTextures.insert( std::pair<SharedTextureType, Ogre::Texture*>(type, shareTexture) );

		return true;
	}

	Ogre::Texture* SharedResources::getTexture( SharedResources::SharedTextureType type )
	{
		SharedTexIterator it = sharedTextures.find(type);
		if ( it == sharedTextures.end() )
			return NULL;

		return it->second;
	}
}
