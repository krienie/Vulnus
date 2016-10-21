
#pragma once

#include <d3d11.h>

#include <OgreD3D11Texture.h>

namespace Vulnus
{
	class WoundErosionFilter
	{
		public:
			WoundErosionFilter( _In_ ID3D11Device* dxDev );
			~WoundErosionFilter();

			void erode( _Inout_ Ogre::D3D11Texture *tex, UINT kernWidth );

		private:
			template<class T> inline void safeRelease(T **ppT)
			{
				if (*ppT)
				{
					(*ppT)->Release();
					*ppT = NULL;
				}
			}

			bool setupKernel();

			ID3D11Device* dxDevice;
			ID3D11DeviceContext *dxDevContext;
			ID3D11ComputeShader* erodeShader;
			bool isInit;
	};
}
