
#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>

#include "Keyframe.h"

namespace Vulnus
{
	class VasodilationFrame : public Keyframe
	{
		public:
			VasodilationFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model );
			//~VasodilationFrame();

			SampledTexture renderFrame();

		private:
			static const std::string PATCH_TEXUNIT_NAME;

			long noiseSeed;
	};
}
