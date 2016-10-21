
#pragma once

#include <vector>

#include "Utils.h"
#include "Renderer.h"

namespace Vulnus
{
	class WoundShapePainter
	{
		public:
			WoundShapePainter( _In_ Renderer *renderer, _In_ Ogre::Texture *upperSkinTexture, _In_ Ogre::Texture *lowerSkinTexture );
			//~WoundShapePainter();

			const std::vector<Utils::HullPoint>* getUVRecordSinkPtr() const;

			bool beginUVRecord( float normMouseX, float normMouseY );
			bool paint( float normMouseX, float normMouseY );
			void end();

			void paintPatch();
			bool clearTextures();

		private:
			bool isOnRightSide( const Utils::HullPoint &l1, const Utils::HullPoint &l2, const Utils::HullPoint &p );
			bool getUVCoord( float normMouseX, float normMouseY, Utils::HullPoint &uvCoord );
			bool fillConvexHull( _In_ const std::vector<Utils::HullPoint> *hull );

			Renderer *ren;
			Ogre::Texture *upperSkinTex;
			Ogre::Texture *lowerSkinTex;

			Utils::HullPoint prevRayPaint;
			bool isRecording;
			std::vector<Utils::HullPoint> uvList;
	};

	bool patchSort( Utils::HullPoint i, Utils::HullPoint j );
}
