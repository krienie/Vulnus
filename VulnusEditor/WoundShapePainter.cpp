
#include <algorithm>
#include <math.h>

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreSubEntity.h>
#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>

#include "Utils.h"
#include "WoundErosionFilter.h"
#include "WoundShapePainter.h"

namespace Vulnus
{
	WoundShapePainter::WoundShapePainter( _In_ Renderer *renderer, _In_ Ogre::Texture *upperSkinTexture, _In_ Ogre::Texture *lowerSkinTexture )
		: ren(renderer), upperSkinTex(upperSkinTexture), lowerSkinTex(lowerSkinTexture), prevRayPaint(0.0f, 0.0f), isRecording(false), uvList()
	{
	}

	/*WoundShapePainter::~WoundShapePainter()
	{
	}*/


	const std::vector<Utils::HullPoint>* WoundShapePainter::getUVRecordSinkPtr() const
	{
		return &uvList;
	}


	bool WoundShapePainter::beginUVRecord( float normMouseX, float normMouseY )
	{
		if ( !upperSkinTex || !lowerSkinTex )
		{
			std::cout << "(WoundShapePainter::beginUVRecord) Error: Paint textures not defined!" << std::endl;
			return false;
		}

		bool hit = getUVCoord( normMouseX, normMouseY, prevRayPaint );
		if ( hit )
		{
			isRecording = true;
			return true;
		}

		return false;
	}


	bool WoundShapePainter::paint( float normMouseX, float normMouseY )
	{
		Ogre::Ray ray;
		Ogre::Camera *cam = ren->getCurrentCamera();
		cam->getCameraToViewportRay( normMouseX, normMouseY, &ray );

		Ogre::Vector2 uvCoord;
		bool hit = ren->getUVRayQuery( ray, uvCoord );

		if ( hit )
		{
			if ( isRecording )
				uvList.push_back(prevRayPaint);

			//TODO: fix this! => no visible feedback for drawing without the drawLine
			Utils::HullPoint curRayPaint = Utils::HullPoint( uvCoord.x, uvCoord.y );
			//Utils::drawLine( upperSkinTex, prevRayPaint, curRayPaint );
			prevRayPaint = curRayPaint;
		}

		return hit;
	}


	void WoundShapePainter::end()
	{
		isRecording = false;
		uvList.push_back(prevRayPaint);
	}


	void WoundShapePainter::paintPatch()
	{
		if ( uvList.size() < 3 )
			return;

		std::sort( uvList.begin(), uvList.end(), patchSort );

		// create upper shell
		std::vector<Utils::HullPoint> lUpper;
		lUpper.push_back( uvList[0] );
		lUpper.push_back( uvList[1] );

		std::vector<Utils::HullPoint>::iterator i;
		for ( i = uvList.begin() + 2; i != uvList.end(); ++i )
		{
			lUpper.push_back( *i );

			while ( lUpper.size() > 2
					&& !isOnRightSide(lUpper[lUpper.size() - 3], lUpper[lUpper.size() - 2], lUpper[lUpper.size() - 1]) )
			{
				lUpper.erase( lUpper.end() - 2 );
			}
		}

		// create lower shell
		std::vector<Utils::HullPoint> lLower;
		lLower.push_back( uvList[ uvList.size() - 1 ] );
		lLower.push_back( uvList[ uvList.size() - 2 ] );

		std::vector<Utils::HullPoint>::reverse_iterator revI;
		for ( revI = uvList.rbegin() + 2; revI != uvList.rend(); ++revI )
		{
			lLower.push_back( *revI );

			while ( lLower.size() > 2
					&& !isOnRightSide(lLower[lLower.size() - 3], lLower[lLower.size() - 2], lLower[lLower.size() - 1]) )
			{
				lLower.erase( lLower.end() - 2 );
			}
		}

		// construct convex hull shell
		lLower.erase( lLower.begin() );
		lLower.erase( lLower.end() - 1 );
		std::vector<Utils::HullPoint> hull(lUpper.begin(), lUpper.end());
		hull.insert( hull.end(), lLower.begin(), lLower.end() );

		uvList = std::vector<Utils::HullPoint>(hull.begin(), hull.end());

		// paint patch
		fillConvexHull( &uvList );
	}


	bool WoundShapePainter::clearTextures()
	{
		if ( !upperSkinTex || !lowerSkinTex )
		{
			std::cout << "(WoundShapePainter::clearTexture) Error: Paint textures not defined!" << std::endl;
			return false;
		}

		// clear upperSkinTexture
		const Ogre::PixelBox *texPB = NULL;
		Utils::lockTextureByPointer( upperSkinTex, &texPB );
		
		Ogre::uint32 *data = static_cast<Ogre::uint32*>(texPB->data);
		size_t texNumPixels = texPB->getWidth() * texPB->getHeight();
		size_t i;
		for ( i = 0; i < texNumPixels; ++i )
				data[i] = 0x0;
		//TODO: use memset for this

		Utils::unlockTextureByPointer( upperSkinTex );

		
		// clear lowerSkinTexture
		Utils::lockTextureByPointer( lowerSkinTex, &texPB );
		
		data = static_cast<Ogre::uint32*>(texPB->data);
		texNumPixels = texPB->getWidth() * texPB->getHeight();
		for ( i = 0; i < texNumPixels; ++i )
				data[i] = 0x0;

		Utils::unlockTextureByPointer( lowerSkinTex );

		return true;
	}


	bool WoundShapePainter::isOnRightSide( const Utils::HullPoint &l1, const Utils::HullPoint &l2, const Utils::HullPoint &p )
	{
		return ((l2.u - l1.u) * (p.v - l1.v)) - ((l2.v - l1.v) * (p.u - l1.u)) < 0;
	}


	bool WoundShapePainter::getUVCoord( float normMouseX, float normMouseY, Utils::HullPoint &uvCoord )
	{
		Ogre::Ray ray;
		Ogre::Camera *cam = ren->getCurrentCamera();
		cam->getCameraToViewportRay( normMouseX, normMouseY, &ray );

		Ogre::Vector2 queryResult;
		bool hit = ren->getUVRayQuery( ray, queryResult );
		uvCoord  = Utils::HullPoint(queryResult.x, queryResult.y);

		return hit;
	}


	bool WoundShapePainter::fillConvexHull( _In_ const std::vector<Utils::HullPoint> *hull )
	{
		const Ogre::PixelBox *upperSkinTexPB;
		Utils::lockTextureByPointer( upperSkinTex, &upperSkinTexPB );

		// find bounding box of convex hull
		UINT bbLeft, bbRight, bbTop, bbBottom;
		bbLeft = bbRight = bbTop = bbBottom = 0;
		Utils::getHullBoundingBox( UINT(upperSkinTexPB->getWidth()), UINT(upperSkinTexPB->getHeight()), &bbLeft, &bbRight, &bbTop, &bbBottom, hull );

		//std::cout << "painting inside!" << std::endl;


		Ogre::uint32 *data = static_cast<Ogre::uint32*>(upperSkinTexPB->data);
		for ( UINT y = bbTop; y < bbBottom; ++y )
		{
			for ( UINT x = bbLeft; x < bbRight; ++x )
			{
				//get UV coordinate of current pixel
				Utils::HullPoint pixP( x / float(upperSkinTexPB->getWidth()), y / float(upperSkinTexPB->getHeight()) );
				
				bool isInHull = true;
				std::vector<Utils::HullPoint>::const_iterator i;
				for ( i = hull->cbegin(); i != hull->cend() - 1; ++i )
				{
					if ( !isOnRightSide(*i, *(i + 1), pixP) )
					{
						isInHull = false;
						break;
					}
				}

				// check last and first
				if ( isInHull )
					isInHull = isOnRightSide( (*hull)[hull->size() - 1], (*hull)[0], pixP );

				if ( isInHull )
					data[int(upperSkinTexPB->rowPitch * y) + x] = 0xFFFFFFFF;
				else data[int(upperSkinTexPB->rowPitch * y) + x] = 0x0;
			}
		}

		Utils::unlockTextureByPointer( upperSkinTex );

		// create destroyed wound edge effect
		Ogre::D3D11RenderSystem *renderSys = dynamic_cast<Ogre::D3D11RenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem());
		WoundErosionFilter erodeKernel( renderSys->_getDevice().get() );
		erodeKernel.erode( dynamic_cast<Ogre::D3D11Texture*>( upperSkinTex ), 5 );

		Ogre::TexturePtr lowerSkinTexPtr = Ogre::TextureManager::getSingletonPtr()->getByName( lowerSkinTex->getName() );
		upperSkinTex->copyToTexture(lowerSkinTexPtr);

		erodeKernel.erode( dynamic_cast<Ogre::D3D11Texture*>( lowerSkinTex ), 10 );

		return true;
	}

	bool patchSort( Utils::HullPoint i, Utils::HullPoint j )
	{
		return i.u < j.u;
	}
}
