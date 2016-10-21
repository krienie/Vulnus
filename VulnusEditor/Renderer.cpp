
#include <sstream>

#include <OgreCamera.h>
#include <OgreConfigFile.h>
#include <OgreEntity.h>
#include <OgreException.h>
#include <OgreRenderWindow.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreSubMesh.h>
#include <OgreMaterialManager.h>
#include <OgreMesh.h>
#include <OgreViewport.h>
#include <OgreString.h>
#include <OgreTechnique.h>
#include <OgreTextureManager.h>

#include "NormalmapGen.h"
#include "NoiseRendererGPU.h"
#include "GradientEdgeRenderer.h"
#include "Renderer.h"

namespace Vulnus
{
	const int Renderer::GLOBAL_TEX_WIDTH  = 2048;
	const int Renderer::GLOBAL_TEX_HEIGHT = 2048;
	const Ogre::ColourValue Renderer::VIEWPORT_CLEAR_COLOR = Ogre::ColourValue(0.5f, 0.5f, 0.5f);

	Renderer::Renderer( HWND winID, QWidget *widgetParent )
		: ogreRoot(0), mainWidget(0), mainCam(0), camTarget(0, 0, 0),
			rayQuery(0), resConfig(""), pluginConfig(""),
			altDown(false), leftMouseDown(false), middleMouseDown(false)
	{
#ifdef _DEBUG
		resConfig    = "resources_d.cfg";
		pluginConfig = "plugins_d.cfg";
#else
		resConfig    = "resources.cfg";
		pluginConfig = "plugins.cfg";
#endif

		ogreRoot = new Ogre::Root(pluginConfig);

		// set up resources
		// Load resource paths from config file
		Ogre::ConfigFile cf;
		cf.load(resConfig);

		// Go through all sections & settings in the file
		Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
 
		Ogre::String secName, typeName, archName;
		while (seci.hasMoreElements())
		{
			secName = seci.peekNextKey();
			Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
			Ogre::ConfigFile::SettingsMultiMap::iterator i;
			for (i = settings->begin(); i != settings->end(); ++i)
			{
				typeName = i->first;
				archName = i->second;
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}


		Ogre::RenderSystem *rs = ogreRoot->getRenderSystemByName("Direct3D11 Rendering Subsystem");
		rs->setConfigOption("Full Screen", "No");
		rs->setConfigOption("Video Mode", "800 x 600 @ 32-bit colour");
		rs->setConfigOption("VSync", "Yes");
		ogreRoot->setRenderSystem(rs);

		ogreRoot->initialise(false);
		mainWidget = new OgreRenderWidget( widgetParent );
		mainWidget->initialise( "mainRenderWindow", winID );
		mainWidget->addInputListener(this);

		// Set default mipmap level (note: some APIs ignore this)
		Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
		// initialise all resource groups
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		Ogre::MaterialManager::getSingletonPtr()->setDefaultTextureFiltering( Ogre::FO_ANISOTROPIC, Ogre::FO_ANISOTROPIC, Ogre::FO_ANISOTROPIC );
		Ogre::MaterialManager::getSingletonPtr()->setDefaultAnisotropy( 16 );

		Ogre::D3D11RenderSystem *renderSys = dynamic_cast<Ogre::D3D11RenderSystem*>(Ogre::Root::getSingletonPtr()->getRenderSystem());
		NoiseRendererGPU::setupRenderer( renderSys->_getDevice().get() );
		NormalmapGen::setupRenderer( renderSys->_getDevice().get() );
		GradientEdgeRenderer::setupRenderer( renderSys->_getDevice().get() );
	}

	Renderer::~Renderer()
	{
		NormalmapGen::cleanup();
		NoiseRendererGPU::cleanup();
		GradientEdgeRenderer::cleanup();

		if ( mainWidget )
			delete mainWidget;

		delete ogreRoot;
	}


	Ogre::Entity* Renderer::loadModel( const std::string &name, const std::string &file, _In_ Ogre::SceneManager *manager )
	{
		Ogre::Entity* modelEntity = manager->createEntity(name, file);
		Ogre::SceneNode *modelNode = manager->getRootSceneNode()->createChildSceneNode();
		modelNode->attachObject( modelEntity );

		return modelEntity;
	}

	Ogre::Material* Renderer::attachMaterial( _In_ Ogre::Entity *model, const std::string &matName )
	{
		Ogre::MaterialPtr matPtr( Ogre::MaterialManager::getSingleton().getByName( matName ) );
		if ( !matPtr.isNull() )
		{
			model->setMaterial(matPtr);
			Ogre::Material *mat = matPtr.get();
			matPtr.setNull();
			return mat;
		}

		return 0;
	}


	Ogre::Texture* Renderer::getManualTexture( const std::string &name, Ogre::TextureUsage usage, 
			Ogre::TextureType type, Ogre::PixelFormat format, UINT depth, int numMipMaps )
	{
		Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().getByName( name );
		if ( texPtr.isNull() )
		{
			texPtr = Ogre::TextureManager::getSingleton().createManual(
				name,														// name
				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				type,														// type
				GLOBAL_TEX_WIDTH, GLOBAL_TEX_HEIGHT,	// width & height
				depth,
				numMipMaps,															// number of mipmaps
				format,														// pixel format
				usage );													// usage; should be TU_DYNAMIC_WRITE_ONLY_DISCARDABLE for
																			// textures updated very often (e.g. each frame)
		}
		Ogre::Texture* tex = texPtr.get();
		texPtr.setNull();

		return tex;
	}


	void Renderer::setupScene()
	{
		Ogre::SceneManager *mainSceneMgr = mainWidget->getSceneManager();
		mainCam = mainSceneMgr->createCamera("MainCam");
		mainCam->setPosition(Ogre::Vector3(20, 20, 20));
		mainCam->lookAt( Ogre::Vector3(0, 0, 0) );
		mainCam->setNearClipDistance( 5 );
		mainCam->setFarClipDistance( 0 );

		Ogre::Viewport* vp = mainWidget->getRenderWindow()->addViewport(mainCam);
		vp->setBackgroundColour( VIEWPORT_CLEAR_COLOR );

		// Alter the camera aspect ratio to match the viewport
		mainCam->setAspectRatio( Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()) );

		mainSceneMgr->setAmbientLight(Ogre::ColourValue(0.25, 0.25, 0.25));
		Ogre::Light* light = mainSceneMgr->createLight("MainDirLight");
		light->setPowerScale( 0.7f );
		light->setDirection( -0.25f, -1.0f, -0.75f );
		light->setPosition( 0, 0, 0 );

		ogreRoot->addFrameListener(this);
	}


	OgreRenderWidget* Renderer::getMainRenderWidget() const
	{
		return mainWidget;
	}

	Ogre::D3D11RenderSystem* Renderer::getRenderSystem() const
	{
		Ogre::RenderSystem *renderSys = ogreRoot->getRenderSystem();
		return dynamic_cast<Ogre::D3D11RenderSystem*>(renderSys);
	}


	Ogre::Camera* Renderer::getCurrentCamera() const
	{
		return mainCam;
	}


	bool Renderer::getUVRayQuery( const Ogre::Ray &ray, Ogre::Vector2 &uvCoord, Ogre::Entity **hitObject )
	{
		if ( !rayQuery )
		{
			// create the ray scene query object
			rayQuery = mainWidget->getSceneManager()->createRayQuery(Ogre::Ray() );
			if ( !rayQuery )
			{
				std::cout << "Failed to create Ogre::RaySceneQuery instance" << std::endl;
				return false;
			}
			rayQuery->setSortByDistance(true);
		}

		rayQuery->setRay(ray);
        if ( rayQuery->execute().size() <= 0 )
            return false;									// raycast did not hit an objects bounding box

		float closestDist = -1.0f;
		Ogre::Vector3 closestRes;
		Ogre::Vector2 uvHitpoint(-1, -1);
		Ogre::RaySceneQueryResult &queryRes = rayQuery->getLastResults();
		Ogre::RaySceneQueryResult::iterator it;
		for ( it = queryRes.begin(); it != queryRes.end(); ++it )
		{
			// stop checking if we have found a raycast hit that is closer
			// than all remaining entities
			if ( (closestDist >= 0.0f) && (closestDist < it->distance) )
				 break;
 
			// only check this result if its a hit against an entity
			if ( it->movable && (it->movable->getMovableType().compare("Entity") == 0) )
			{
				// get the entity to check
				Ogre::Entity *ent = static_cast<Ogre::Entity*>(it->movable);      

				// mesh data to retrieve         
				size_t vertCount;
				size_t idxCount;
				Ogre::Vector3 *vertices;
				ULONG *indices;
				Ogre::Vector2 *uvCoords;

				Ogre::Vector3 pos       = ent->getParentNode()->convertLocalToWorldPosition( ent->getParentNode()->getPosition() );
				Ogre::Quaternion orient = ent->getParentNode()->convertLocalToWorldOrientation( ent->getParentNode()->getOrientation() );
				Ogre::Vector3 scale     = ent->getParentNode()->_getDerivedScale();
 
				// get the mesh information
				getMeshInformation( ent->getMesh(), pos, orient, scale,
										vertCount, vertices, idxCount, indices, uvCoords );

				// test for hitting individual triangles on the mesh
				bool newClosestFound = false;
				int hitIndex;
				for ( int i = 0; i < static_cast<int>(idxCount); i += 3 )
				{
					// check for a hit against this triangle
					std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray, vertices[indices[i]],
														vertices[indices[i+1]], vertices[indices[i+2]], true, false);
 
					// if it was a hit check if its the closest
					if ( hit.first && ((closestDist < 0.0f) || (hit.second < closestDist)) )
					{
						// this is the closest so far, save it off
						closestDist     = hit.second;
						newClosestFound = true;
						hitIndex        = i;
					}
				}
 
				// if we found a new closest raycast for this object, update the
				// closest_result before moving on to the next object.
				if ( newClosestFound )
				{
					// calculate UV hitpoint
					closestRes = ray.getPoint(closestDist);

					Ogre::Vector3 u = vertices[indices[hitIndex+1]] - vertices[indices[hitIndex]];
					Ogre::Vector3 v = vertices[indices[hitIndex+2]] - vertices[indices[hitIndex]];
					Ogre::Vector3 w = closestRes - vertices[indices[hitIndex]];

					float denom = u.crossProduct(v).length();
					float r = v.crossProduct(w).length() / denom;
					float t = u.crossProduct(w).length() / denom;
					float s = 1 - r - t;

					uvHitpoint = ( s * uvCoords[indices[hitIndex]] )
									+ ( r * uvCoords[indices[hitIndex+1]] )
									+ ( t * uvCoords[indices[hitIndex+2]] );

					// clamp to [0 - 1]
					if ( uvHitpoint.x > 1.0f )
						uvHitpoint.x -= int(uvHitpoint.x);
					else if ( uvHitpoint.x < 0.0f )
					{
						int add = int(uvHitpoint.x - 1.0f);
						uvHitpoint.x += Ogre::Math::Abs( Ogre::Real(add) );
					}

					if ( uvHitpoint.y > 1.0f )
						uvHitpoint.y -= int(uvHitpoint.y);
					else if ( uvHitpoint.y < 0.0f )
					{
						int add = int(uvHitpoint.y - 1.0f);
						uvHitpoint.y += Ogre::Math::Abs( Ogre::Real(add) );
					}

					//save hit object
					if ( hitObject )
						*hitObject = ent;
				}

				// free the verticies and indicies memory
				delete[] vertices;
				delete[] indices;
				delete[] uvCoords;
			}       
		}

		// return the result
		if ( closestDist >= 0.0f )
		{
			uvCoord = uvHitpoint;
			return true;
		}
		
		uvCoord = Ogre::Vector2(-1.0f, -1.0f);
		return false;
	}


	void Renderer::mouseMoved( const MouseEvent &evt )
	{
		if ( middleMouseDown )
		{
			if ( altDown )			// rotate scene
			{
				Ogre::SceneManager *sceneMgr = mainWidget->getSceneManager();
				Ogre::SceneNode *rootNode    = sceneMgr->getRootSceneNode();

				float rotate = 0.5f;
				Ogre::Quaternion qYrot( Ogre::Degree(rotate * -evt.X().rel()), Ogre::Vector3(0.0f, 1.0f, 0.0f) );
				rootNode->rotate( qYrot, Ogre::Node::TS_LOCAL );

				Ogre::Quaternion qXrot( Ogre::Degree(rotate * -evt.Y().rel()), mainCam->getRight() );
				rootNode->rotate( qXrot, Ogre::Node::TS_WORLD );
			} else					// pan camera
			{
				Ogre::Vector3 realCamUp = mainCam->getRight().crossProduct( mainCam->getDirection() );
				Ogre::Vector3 newRelPos = ( (mainCam->getRight() * evt.X().rel()) - (realCamUp * evt.Y().rel()) ) * 0.05f;

				mainCam->setPosition( newRelPos + mainCam->getPosition() );
				camTarget += newRelPos;
				mainCam->lookAt( camTarget );
			}
		}

		if ( evt.Z().abs() != 0 )	// zoom camera
		{
			Ogre::Vector3 distVect = camTarget - mainCam->getPosition();

			distVect.normalise();
			distVect *= evt.Z().abs() * 0.025f;

			mainCam->setPosition( mainCam->getPosition() + distVect );
		}
	}

	void Renderer::mouseEvent( const MouseEvent &evt )
	{
		if ( evt.button() == MouseEvent::Button::LeftButton )
			leftMouseDown = evt.state() == ButtonState::Down;

		if ( evt.button() == MouseEvent::Button::MiddleButton )
			middleMouseDown = evt.state() == ButtonState::Down;
	}

	void Renderer::keyboardEvent( _In_ QKeyEvent *evt, const ButtonState &state )
	{
		if ( evt->key() == Qt::Key::Key_Alt )
			altDown = state == ButtonState::Down;
	}


	void Renderer::getMeshInformation( const Ogre::MeshPtr mesh, const Ogre::Vector3 &position, const Ogre::Quaternion &orient,
											const Ogre::Vector3 &scale, size_t &vertCount, _Out_ Ogre::Vector3* &vertices,
											size_t &idxCount, _Out_ ULONG* &indices, _Out_ Ogre::Vector2* &uvCoords )
	{
		bool added_shared     = false;
		size_t current_offset = 0;
		size_t shared_offset  = 0;
		size_t next_offset    = 0;
		size_t index_offset   = 0;
 
		vertCount = idxCount = 0;
 
		// Calculate how many vertices and indices we're going to need
		for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
		{
			Ogre::SubMesh* submesh = mesh->getSubMesh( i );
 
			// We only need to add the shared vertices once
			if( submesh->useSharedVertices && !added_shared )
			{
				vertCount += mesh->sharedVertexData->vertexCount;
				added_shared = true;
			} else
			{
				vertCount += submesh->vertexData->vertexCount;
			}
 
			// Add the indices
			idxCount += submesh->indexData->indexCount;
		}


		// Allocate space for the vertices and indices
		vertices = new Ogre::Vector3[vertCount];
		indices  = new ULONG[idxCount];
		uvCoords = new Ogre::Vector2[vertCount];
 
		added_shared = false;
 
		// Run through the submeshes again, adding the data into the arrays
		for ( unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
		{
			Ogre::SubMesh* submesh = mesh->getSubMesh(i);
 
			Ogre::VertexData* vertData = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
 
			if( (!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared) )
			{
				if(submesh->useSharedVertices)
				{
					added_shared = true;
					shared_offset = current_offset;
				}
 
				const Ogre::VertexElement* posElem =
					vertData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
				const Ogre::VertexElement* uvElem =
					vertData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
 
				Ogre::HardwareVertexBufferSharedPtr vbuf =
					vertData->vertexBufferBinding->getBuffer(posElem->getSource());
 
				unsigned char* vertex =
					static_cast<UCHAR*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
 
				// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
				//  as second argument. So make it float, to avoid trouble when Ogre::Real will
				//  be compiled/typedefed as double:
				//      Ogre::Real* pReal;
				float* pReal;
				float* pUV;
 
				for( size_t j = 0; j < vertData->vertexCount; ++j, vertex += vbuf->getVertexSize())
				{
					posElem->baseVertexPointerToElement(vertex, &pReal);
					uvElem->baseVertexPointerToElement(vertex, &pUV);

					Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);

					vertices[current_offset + j] = (orient * (pt * scale)) + position;
					uvCoords[current_offset + j] = Ogre::Vector2( pUV[0], pUV[1] );
				}
 
				vbuf->unlock();
				next_offset += vertData->vertexCount;
			}


			Ogre::IndexData* index_data = submesh->indexData;
			size_t numTris = index_data->indexCount / 3;
			Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
			if( ibuf.isNull() )
				continue; // need to check if index buffer is valid (which will be not if the mesh doesn't have triangles like a pointcloud)
 
			bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
 
			ULONG *pLong   = static_cast<ULONG*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			USHORT *pShort = reinterpret_cast<USHORT*>(pLong);
 
 
			size_t offset = (submesh->useSharedVertices)? shared_offset : current_offset;
			size_t index_start = index_data->indexStart;
			size_t last_index = numTris*3 + index_start;
 
			if ( use32bitindexes )
				for (size_t k = index_start; k < last_index; ++k)
					indices[index_offset++] = pLong[k] + static_cast<ULONG>( offset );
			else
				for (size_t k = index_start; k < last_index; ++k)
					indices[ index_offset++ ] = static_cast<ULONG>( pShort[k] ) + static_cast<ULONG>( offset );
 
			ibuf->unlock();
			current_offset = next_offset;
		}
	}

	bool Renderer::frameRenderingQueued( const Ogre::FrameEvent& e )
	{
		if( mainWidget->getRenderWindow()->isClosed() )
			return false;

		return true;
	}
}
