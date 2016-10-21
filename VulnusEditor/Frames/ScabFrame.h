
#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>

#include "Keyframe.h"

namespace Vulnus
{
	class ScabFrame : public Keyframe
	{
		public:
			ScabFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model );
			//~ScabFrame();

			SampledTexture renderFrame();
			SampledTexture sample( double position );

		private:
			long noiseSeed;
	};
}
