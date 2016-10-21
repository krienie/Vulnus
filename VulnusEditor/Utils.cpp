
#include <OgreHardwarePixelBuffer.h>
#include <OgreTechnique.h>
#include <OgreMaterialManager.h>
#include <OgreTextureManager.h>

#include "Utils.h"

namespace Vulnus { namespace Utils
{
	bool lineLineIsect( const Ogre::Vector2 &p1, const Ogre::Vector2 &p2,
											const Ogre::Vector2 &p3, const Ogre::Vector2 &p4, Ogre::Vector2 &isect )
	{
		Ogre::Vector2 r = p2 - p1;
		Ogre::Vector2 s = p4 - p3;
				
		Ogre::Vector2 temp = p3 - p1;

		float t = temp.crossProduct(s);
		t /= r.crossProduct(s);

		if ( t < 0 )
			return false;

		float u = temp.crossProduct(r);
		u /= r.crossProduct(s);

		if ( u < 0.0f || u > 1.0f )
			return false;

		isect = p1 + t * r;
		return true;
	}

	float distFromLineSegment( const Ogre::Vector2 &l1, const Ogre::Vector2 &l2, const Ogre::Vector2 &p )
	{
		Ogre::Vector2 line = l2 - l1;
		line = Ogre::Vector2( line.y, -line.x );
		line.normalise();
		Ogre::Vector2 pToLine = l1 - p;

		float dot = (p - l2).dotProduct(l2 - l1);
		if ( dot > 0 )
		{
			Ogre::Vector2 l2MinP = l2 - p;
			return std::sqrtf( l2MinP.dotProduct(l2MinP) );
		}

		dot = (p - l1).dotProduct(l1 - l2); 
		if ( dot > 0 )
		{
			Ogre::Vector2 l1MinP = l1 - p;
			return std::sqrtf( l1MinP.dotProduct(l1MinP) );
		}

		return std::abs(line.dotProduct(pToLine));
	}

	void getHullBoundingBox( UINT pbWidth, UINT pbHeight, _Inout_ UINT *left, _Inout_ UINT *right,
								_Inout_ UINT *top, _Inout_ UINT *bottom, _In_ const std::vector<HullPoint> *hull )
	{
		*left   = UINT((*hull)[0].u * pbWidth);
		*right  = UINT((*hull)[0].u * pbWidth) + 1;
		*top    = UINT((*hull)[0].v * pbHeight);
		*bottom = UINT((*hull)[0].v * pbHeight) + 1;

		std::vector<HullPoint>::const_iterator i;
		for ( i = hull->cbegin() + 1; i != hull->cend(); ++i )
		{
			UINT tempPos = UINT(i->u * pbWidth);
			if ( *left > tempPos )
				*left = tempPos;
			else if ( *right < tempPos )
				*right = tempPos;

			tempPos = int(i->v * pbHeight);
			if ( *top > tempPos )
				*top = tempPos;
			else if ( *bottom < tempPos )
				*bottom = tempPos;
		}
	}

	void lockTextureByName( const std::string &texName, _Inout_ const Ogre::PixelBox **pb )
	{
		if ( !pb )
		{
			std::cout << "(Utils::lockTextureByName) Error: No PixelBox pointer supplied" << std::endl;
			return;
		}

		Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().getByName( texName );
		Ogre::Texture *tex = texPtr.get();
		texPtr.setNull();

		if ( !tex )
		{
			std::cout << "(Utils::lockTextureByName) Error: Texture " << texName << " was not found. Unable to lock." << std::endl;
			return;
		}

		Ogre::HardwarePixelBuffer *pixBuff = tex->getBuffer().get();
		pixBuff->lock(0, pixBuff->getSizeInBytes(), Ogre::HardwareBuffer::LockOptions::HBL_NORMAL);
		*pb = &pixBuff->getCurrentLock();
	}

	void lockTextureByPointer( _In_ Ogre::Texture* tex, _Out_ const Ogre::PixelBox **pb )
	{
		if ( !pb )
		{
			std::cout << "(Utils::lockTextureByName) Error: No PixelBox pointer supplied" << std::endl;
			return;
		}

		Ogre::HardwarePixelBuffer *pixBuff = tex->getBuffer().get();
		pixBuff->lock(0, pixBuff->getSizeInBytes(), Ogre::HardwareBuffer::LockOptions::HBL_NORMAL);
		*pb = &pixBuff->getCurrentLock();
	}


	void unlockTextureByName( const std::string &texName )
	{
		Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().getByName( texName );
		Ogre::Texture *tex = texPtr.get();
		texPtr.setNull();

		tex->getBuffer().get()->unlock();
	}

	void unlockTextureByPointer( _In_ Ogre::Texture* tex )
	{
		tex->getBuffer().get()->unlock();
	}


	void drawLine( _Inout_ Ogre::Texture *drawTex, const HullPoint &p1, const HullPoint &p2, float thickness )
	{
		drawLine( drawTex, Ogre::Vector2(p1.u, p1.v), Ogre::Vector2(p2.u, p2.v), thickness );
	}

	void drawLine( _Inout_ Ogre::Texture *drawTex, const Ogre::Vector2 &p1, const Ogre::Vector2 &p2, float thickness )
	{
		Ogre::HardwarePixelBuffer *pixBuff = drawTex->getBuffer().get();

		pixBuff->lock(0, pixBuff->getSizeInBytes(), Ogre::HardwareBuffer::LockOptions::HBL_NORMAL);
		const Ogre::PixelBox &pb = pixBuff->getCurrentLock();
		
		drawLine(&pb, p1, p2, thickness);

		pixBuff->unlock();
	}

	void drawLine( _Inout_ const Ogre::PixelBox *pb, const Ogre::Vector2 &p1, const Ogre::Vector2 &p2, float thickness )
	{
		//find bounding box
		int xP1 = int(p1.x * pb->getWidth());
		int yP1 = int(p1.y * pb->getHeight());
		int xP2 = int(p2.x * pb->getWidth());
		int yP2 = int(p2.y * pb->getHeight());

		int bbLeft   = std::min(xP1, xP2);
		int bbRight  = std::max(xP1, xP2) + 1;
		int bbTop    = std::min(yP1, yP2);
		int bbBottom = std::max(yP1, yP2) + 1;

		float lineThick = std::max(1.0f, thickness) * 0.5f * ( 1.0f / float(pb->getWidth()) );

		Ogre::uint32 *data = static_cast<Ogre::uint32*>(pb->data);
		for ( int y = bbTop; y < bbBottom; ++y )
		{
			for ( int x = bbLeft; x < bbRight; ++x )
			{
				//get UV coordinate of current pixel
				Ogre::Vector2 pixP( x / float(pb->getWidth()), y / float(pb->getHeight()) );
				if ( Utils::distFromLineSegment( p1, p2, pixP ) <= lineThick + 0.001f )
					data[int(pb->rowPitch * y) + x] = 0xFFFF0000;
			}
		}
	}

	Ogre::GpuProgramParameters* getVertexParameters( const std::string &material )
	{
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( material );
		if ( matPtr.isNull() )
		{
			std::cout << "(WoundPhase::getFragmentParameters) Error: Material " << material << " was not found!" << std::endl;
			return NULL;
		}

		Ogre::GpuProgramParametersSharedPtr gpuProgramParamsPtr = matPtr->getTechnique(0)->getPass(0)->getVertexProgramParameters();
		matPtr.setNull();

		Ogre::GpuProgramParameters *gpuProgramParams = gpuProgramParamsPtr.get();
		gpuProgramParamsPtr.setNull();

		return gpuProgramParams;
	}

	Ogre::GpuProgramParameters* getHullParameters( const std::string &material )
	{
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( material );
		if ( matPtr.isNull() )
		{
			std::cout << "(WoundPhase::getFragmentParameters) Error: Material " << material << " was not found!" << std::endl;
			return NULL;
		}

		Ogre::GpuProgramParametersSharedPtr gpuProgramParamsPtr = matPtr->getTechnique(0)->getPass(0)->getTessellationHullProgramParameters();
		matPtr.setNull();

		Ogre::GpuProgramParameters *gpuProgramParams = gpuProgramParamsPtr.get();
		gpuProgramParamsPtr.setNull();

		return gpuProgramParams;
	}

	Ogre::GpuProgramParameters* getDomainParameters( const std::string &material )
	{
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( material );
		if ( matPtr.isNull() )
		{
			std::cout << "(WoundPhase::getFragmentParameters) Error: Material " << material << " was not found!" << std::endl;
			return NULL;
		}

		Ogre::GpuProgramParametersSharedPtr gpuProgramParamsPtr = matPtr->getTechnique(0)->getPass(0)->getTessellationDomainProgramParameters();
		matPtr.setNull();

		Ogre::GpuProgramParameters *gpuProgramParams = gpuProgramParamsPtr.get();
		gpuProgramParamsPtr.setNull();

		return gpuProgramParams;
	}

	Ogre::GpuProgramParameters* getFragmentParameters( const std::string &material )
	{
		Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().getByName( material );
		if ( matPtr.isNull() )
		{
			std::cout << "(WoundPhase::getFragmentParameters) Error: Material " << material << " was not found!" << std::endl;
			return NULL;
		}

		Ogre::GpuProgramParametersSharedPtr gpuProgramParamsPtr = matPtr->getTechnique(0)->getPass(0)->getFragmentProgramParameters();
		matPtr.setNull();

		Ogre::GpuProgramParameters *gpuProgramParams = gpuProgramParamsPtr.get();
		gpuProgramParamsPtr.setNull();

		return gpuProgramParams;
	}
}}