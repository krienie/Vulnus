
#pragma once

#include <d3d11.h>
#include <random>

#include <OgreD3D11Texture.h>

#include "Utils.h"

namespace Vulnus
{
	class GradientEdgeRenderer
	{
		public:
			enum RenderSide
			{
				Side_Inner,
				Side_Outer,
				Side_Both
			};


			static void setupRenderer( _In_ ID3D11Device* dxDev );
			static void cleanup();
			static void fadeTo( _Inout_ Ogre::D3D11Texture *outputTexture, _In_ Ogre::D3D11Texture *woundShapeTexture,
									_In_ const std::vector<Utils::HullPoint> *hull, RenderSide renderSide, const Ogre::Vector4 &fromClr, UINT thickness = 100U );
			static void fadeTo( _Inout_ Ogre::D3D11Texture *outputTexture, _In_ Ogre::D3D11Texture *woundShapeTexture,
									_In_ const std::vector<Utils::HullPoint> *hull, RenderSide renderSide, const Ogre::Vector4 &fromClr,
									const Ogre::Vector4 &toClr, UINT thickness = 100U );

		private:
			struct FadeShaderParams
			{
				UINT hullBuffSize;
				UINT renderSide;
				UINT thickness;
				bool fadeToTexture;
				float fromColor[4];
				float toColor[4];
			};


			GradientEdgeRenderer();
			~GradientEdgeRenderer();
			GradientEdgeRenderer( const GradientEdgeRenderer& );
			GradientEdgeRenderer& operator=( const GradientEdgeRenderer& );

			static bool setupShaders();
			static void setupForRendering( _In_ Ogre::D3D11Texture *inputTexture, _In_ const std::vector<Utils::HullPoint> *hull );

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
			static ID3D11ComputeShader *fadeShader;
			static ID3D11Buffer *fadeShaderContantBuff;
			static ID3D11Texture2D *renderTex;
			static ID3D11UnorderedAccessView *renderTexUAV;
			static ID3D11Buffer *hullBuff;
			static ID3D11ShaderResourceView *hullSRV;
			static bool isInit;
			static bool fadeToTexture;
	};
}
