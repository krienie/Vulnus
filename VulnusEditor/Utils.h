
#pragma once

#include <OgreVector2.h>

namespace Vulnus { namespace Utils
{
	struct HullPoint
	{
		HullPoint( float _u, float _v ) : u(_u), v(_v) { }

		float u;
		float v;
	};

	bool lineLineIsect( const Ogre::Vector2 &p1, const Ogre::Vector2 &p2, const Ogre::Vector2 &p3, const Ogre::Vector2 &p4, Ogre::Vector2 &isect );
	float distFromLineSegment( const Ogre::Vector2 &l1, const Ogre::Vector2 &l2, const Ogre::Vector2 &p );
	void getHullBoundingBox( UINT pbWidth, UINT pbHeight, _Inout_ UINT *left, _Inout_ UINT *right,
								_Inout_ UINT *top, _Inout_ UINT *bottom, _In_ const std::vector<HullPoint> *hull );
	void lockTextureByName( const std::string &texName, _Out_ const Ogre::PixelBox **pb );
	void lockTextureByPointer( _In_ Ogre::Texture* tex, _Out_ const Ogre::PixelBox **pb );
	void unlockTextureByName( const std::string &texName );
	void unlockTextureByPointer( _In_ Ogre::Texture* tex );
	void drawLine( _Inout_ Ogre::Texture *drawTex, const HullPoint &p1, const HullPoint &p2, float thickness = 1.0f );
	void drawLine( _Inout_ Ogre::Texture *drawTex, const Ogre::Vector2 &p1, const Ogre::Vector2 &p2, float thickness = 1.0f );
	void drawLine( _Inout_ const Ogre::PixelBox *pb, const Ogre::Vector2 &p1, const Ogre::Vector2 &p2, float thickness = 1.0f );
	Ogre::GpuProgramParameters* getVertexParameters( const std::string &material );
	Ogre::GpuProgramParameters* getHullParameters( const std::string &material );
	Ogre::GpuProgramParameters* getDomainParameters( const std::string &material );
	Ogre::GpuProgramParameters* getFragmentParameters( const std::string &material );
}}
