
#pragma once

#include <sal.h>

#include <OgreD3D11RenderSystem.h>
#include <OgreFrameListener.h>
#include <OgreRoot.h>
#include <OgreSceneQuery.h>

#include "InputListener.h"
#include "OgreRenderWidget.h"

namespace Vulnus
{
	class Renderer
		: public Ogre::FrameListener, public InputListener
	{
		public:
			static const int GLOBAL_TEX_WIDTH;
			static const int GLOBAL_TEX_HEIGHT;
			static const Ogre::ColourValue VIEWPORT_CLEAR_COLOR;

			Renderer( HWND winID, QWidget *widgetParent );
			~Renderer();

			static Ogre::Entity* loadModel( const std::string &name, const std::string &file, _In_ Ogre::SceneManager *manager );
			static Ogre::Material* attachMaterial( _In_ Ogre::Entity *model, const std::string &matName );
			static Ogre::Texture* getManualTexture( const std::string &name, Ogre::TextureUsage usage, 
						Ogre::TextureType type = Ogre::TEX_TYPE_2D, Ogre::PixelFormat format = Ogre::PF_BYTE_RGBA, UINT depth = 1, int numMipMaps = 0 );

			void setupScene();
			OgreRenderWidget* getMainRenderWidget() const;
			Ogre::D3D11RenderSystem* getRenderSystem() const;
			Ogre::Camera* getCurrentCamera() const;
			bool getUVRayQuery( const Ogre::Ray &ray, Ogre::Vector2 &uvCoord, Ogre::Entity **hitObject = 0 );

			void frameUpdate( const Ogre::FrameEvent& e ) { }
			void mouseMoved( const MouseEvent &evt );
			void mouseEvent( const MouseEvent &evt );
			void keyboardEvent( _In_ QKeyEvent *evt, const ButtonState &state );

		private:
			void getMeshInformation( const Ogre::MeshPtr mesh, const Ogre::Vector3 &position, const Ogre::Quaternion &orient,
										const Ogre::Vector3 &scale, size_t &vertCount, _Out_ Ogre::Vector3* &vertices,
										size_t &idxCount, _Out_ ULONG* &indices, _Out_ Ogre::Vector2* &uvCoords );

			Ogre::Vector3 getBarycentricCoords( const Ogre::Vector2 &v1, const Ogre::Vector2 &v2, const Ogre::Vector2 &v3, const Ogre::Vector2 &p );

			bool frameRenderingQueued( const Ogre::FrameEvent& evt );


			Ogre::Root *ogreRoot;
			OgreRenderWidget *mainWidget;

			Ogre::Camera *mainCam;
			Ogre::Vector3 camTarget;

			Ogre::RaySceneQuery *rayQuery;

			Ogre::String resConfig;
			Ogre::String pluginConfig;

			bool altDown;
			bool leftMouseDown;
			bool middleMouseDown;
	};
}
