
#pragma once

#include <d3d11.h>
#include <random>

#include <OgreD3D11Texture.h>

namespace Vulnus
{
	class NoiseRendererGPU
	{
		public:
			static void setupRenderer( _In_ ID3D11Device* dxDev );
			static void cleanup();
			static long getNewSeed();
			static void setSeed( long s );
			
			//TODO: heightDiff is a quick fix for adding height to ScabFrame heightmap. Will prob. create something nicer later
			static long create2DValueNoise( _Inout_ Ogre::D3D11Texture *texture, int octaves = 6, float persistance = 0.5f,
												float bias = 0.5f, float gain = 0.5f, long s = -1l, int stretchY = 1, float heightDiff = 0.0f );

			static long createMaturationFrameNoise( _Inout_ Ogre::D3D11Texture *texture, long s = -1l );

		private:
			struct SmoothNoiseParams
			{
				float frequency;
				float amplitude;
				int stretchY;
				int period;
				UINT imgWidth;
				UINT imgHeight;
			};

			struct NoiseAvgParams
			{
				float bias;
				float gain;
				float totalAmplitude;
				float heightDiff;
			};

			NoiseRendererGPU();
			~NoiseRendererGPU();
			NoiseRendererGPU( const NoiseRendererGPU& );
			NoiseRendererGPU& operator=( const NoiseRendererGPU& );

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
			static ID3D11ComputeShader *smoothNoiseShader;
			static ID3D11ComputeShader *noiseAvgShader;
			static ID3D11ComputeShader *maturationNoiseShader;
			static ID3D11Buffer *smoothNoiseParamsBuff;
			static ID3D11Buffer *noiseAvgParamsBuff;
			static bool isInit;
			static long seed;

			static std::minstd_rand0 randGen;
	};
}
