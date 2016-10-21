
#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>

#include "Keyframe.h"

namespace Vulnus
{
	class VasoconstrictionFrame : public Keyframe
	{
		public:
			VasoconstrictionFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model );
			//~VasoconstrictionFrame();

			SampledTexture renderFrame();

		private:
	};
}
