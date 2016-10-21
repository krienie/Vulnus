
#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>

#include "Keyframe.h"

namespace Vulnus
{
	class PreScabFrame : public Keyframe
	{
		public:
			PreScabFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model );
			//~PreScabFrame();

			SampledTexture renderFrame();

		private:
			long noiseSeed;
	};
}
