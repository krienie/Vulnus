
#pragma once

#include <OgreCamera.h>
#include <OgreEntity.h>

#include "Keyframe.h"

namespace Vulnus
{
	class MaturationFrame : public Keyframe
	{
		public:
			MaturationFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model, _In_ ID3D11Device* dxDev );
			~MaturationFrame();

			SampledTexture renderFrame();
			SampledTexture sample( double position );

		private:
			struct GradientShaderParams
			{
				//float innerWoundBorder;
				//float midWoundBorder;
				float phaseThickness;
				float innerWoundBorder;
				float outerWoundBorder;
				UINT hullBuffSize;
			};


			void recalculateClosingDistance();
			void generateWoundColor( double position );
			void createRenderTexture( _Inout_ ID3D11Texture2D **renderTex, _Inout_ ID3D11UnorderedAccessView **renderTexUAV );
			bool setupRenderResources();

			template<class T> static void safeRelease(T **ppT)
			{
				if (*ppT)
				{
					(*ppT)->Release();
					*ppT = NULL;
				}
			}

			long noiseSeed;
			float closingDistance;

			bool resourcesSetup;
			ID3D11Device *dxDevice;
			ID3D11DeviceContext *dxDevContext;
			ID3D11ComputeShader *gradientShader;
			ID3D11Buffer *gradientShaderContantBuff;

			// textures
			Ogre::D3D11Texture *reflectionNoiseTex;
			Ogre::D3D11Texture *reflectionNoiseMaskTex;
			ID3D11Texture2D *albedoRenderTex;
			ID3D11Texture2D *normalRenderTex;
			ID3D11Texture2D *reflectRenderTex;
			ID3D11UnorderedAccessView *albedoRenderTexUAV;
			ID3D11UnorderedAccessView *normalRenderTexUAV;
			ID3D11UnorderedAccessView *reflectRenderTexUAV;
	};
}
