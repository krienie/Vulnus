#pragma once

#include <d3d11.h>
#include <random>

#include <OgreD3D11Texture.h>

namespace Vulnus
{
	class NormalmapGen
	{
		public:
			static void setupRenderer( _In_ ID3D11Device* dxDev );
			static void cleanup();
			static void fromHeightmap( _Inout_ Ogre::D3D11Texture *texture );

		private:
			NormalmapGen();
			~NormalmapGen();
			NormalmapGen( const NormalmapGen& );
			NormalmapGen& operator=( const NormalmapGen& );

			static bool setupShaders();

			template<class T> static void safeRelease(T **ppT)
			{
				if (*ppT)
				{
					(*ppT)->Release();
					*ppT = NULL;
				}
			}

			static ID3D11Device *dxDevice;
			static ID3D11DeviceContext *dxDevContext;
			static ID3D11ComputeShader *heightToNormalShader;
			static bool isInit;
	};
}
