
#include <chrono>
#include <d3dcompiler.h>

#include "NoiseRendererGPU.h"

namespace Vulnus
{
	ID3D11Device* NoiseRendererGPU::dxDevice                     = 0;
	ID3D11DeviceContext* NoiseRendererGPU::dxDevContext          = 0;
	ID3D11ComputeShader* NoiseRendererGPU::smoothNoiseShader     = 0;
	ID3D11ComputeShader* NoiseRendererGPU::noiseAvgShader        = 0;
	ID3D11ComputeShader* NoiseRendererGPU::maturationNoiseShader = 0;
	ID3D11Buffer* NoiseRendererGPU::smoothNoiseParamsBuff        = 0;
	ID3D11Buffer* NoiseRendererGPU::noiseAvgParamsBuff           = 0;
	bool NoiseRendererGPU::isInit                                = false;
	long NoiseRendererGPU::seed                                  = -1;
	std::minstd_rand0 NoiseRendererGPU::randGen                  = std::minstd_rand0();


	void NoiseRendererGPU::setupRenderer( _In_ ID3D11Device* dxDev )
	{
		dxDevice = dxDev;
		dxDevice->GetImmediateContext( &dxDevContext );
		setSeed(-1);
	}

	void NoiseRendererGPU::cleanup()
	{
		safeRelease(&maturationNoiseShader);
		safeRelease(&noiseAvgParamsBuff);
		safeRelease(&noiseAvgShader);
		safeRelease(&smoothNoiseParamsBuff);
		safeRelease(&smoothNoiseShader);
		safeRelease(&dxDevContext);
	}


	long NoiseRendererGPU::getNewSeed()
	{
		return long(std::chrono::system_clock::now().time_since_epoch().count());
	}

	void NoiseRendererGPU::setSeed( long s )
	{
		if ( s < 0 )
			seed = long(std::chrono::system_clock::now().time_since_epoch().count());
		else seed = s;

		randGen = std::minstd_rand0(seed);
	}


	//TODO: heightDiff is a quick fix for adding height to ScabFrame heightmap. Will prob. create something nicer later
	long NoiseRendererGPU::create2DValueNoise( _Inout_ Ogre::D3D11Texture *texture, int octaves, float persistance,
													float bias, float gain, long s, int stretchY, float heightDiff )
	{
		if ( !isInit && !setupShaders() )
			return s;

		// set seed
		setSeed( s );

		ID3D11Texture2D *noiseOutputTex = texture->GetTex2D();
		D3D11_TEXTURE2D_DESC noiseOutputTexDesc;
		noiseOutputTex->GetDesc( &noiseOutputTexDesc );

		// create render output texture
		D3D11_TEXTURE2D_DESC renTex2DDesc;
		ZeroMemory( &renTex2DDesc, sizeof(renTex2DDesc) );
		renTex2DDesc.Width              = noiseOutputTexDesc.Width;
		renTex2DDesc.Height             = noiseOutputTexDesc.Height;
		renTex2DDesc.MipLevels          = noiseOutputTexDesc.MipLevels;
		renTex2DDesc.ArraySize          = noiseOutputTexDesc.ArraySize;
		renTex2DDesc.SampleDesc.Count   = noiseOutputTexDesc.SampleDesc.Count;
		renTex2DDesc.SampleDesc.Quality = noiseOutputTexDesc.SampleDesc.Quality;
		renTex2DDesc.Usage              = D3D11_USAGE_DEFAULT;
		renTex2DDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
		renTex2DDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

		ID3D11Texture2D *renderTex;
		HRESULT res = dxDevice->CreateTexture2D( &renTex2DDesc, NULL, &renderTex );
		if ( FAILED(res) )
		{
			std::cout << "NoiseRendererGPU::create2DValueNoise aborted: Error creating render texture" << std::endl;
			return seed;
		}

		// create render texture UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		uavDesc.Format             = renTex2DDesc.Format;
		uavDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		ID3D11UnorderedAccessView *renderTexUAV;
		res = dxDevice->CreateUnorderedAccessView( renderTex, &uavDesc, &renderTexUAV );
		if ( FAILED(res) )
		{
			safeRelease(&renderTex);
			std::cout << "NoiseRendererGPU::create2DValueNoise aborted: Error creating unordered access view for render texture" << std::endl;
			return seed;
		}


		// initialize Value Noise buffer to 0.0f
		const UINT STRUCTBUFF_SIZE = noiseOutputTexDesc.Width * noiseOutputTexDesc.Height;
		std::vector<float> valueNoiseData(STRUCTBUFF_SIZE, 0.0f);
			
		D3D11_SUBRESOURCE_DATA subResourceData;
		ZeroMemory( &subResourceData, sizeof( subResourceData ) );
		subResourceData.pSysMem = valueNoiseData.data();

		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.BindFlags           = D3D11_BIND_UNORDERED_ACCESS;
		buffDesc.ByteWidth           = STRUCTBUFF_SIZE * sizeof(float);
		buffDesc.StructureByteStride = sizeof(float);
		buffDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		buffDesc.Usage               = D3D11_USAGE_DEFAULT;
		ID3D11Buffer *valueNoiseBuff;
		res = dxDevice->CreateBuffer(&buffDesc, &subResourceData, &valueNoiseBuff);
		if ( FAILED(res) )
		{	
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "NoiseRendererGPU::create2DValueNoise aborted: Error creating structured buffer" << std::endl;
			return seed;
		}

		// create Value Noise UAV
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		uavDesc.Format              = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements  = STRUCTBUFF_SIZE;
		ID3D11UnorderedAccessView *valueNoiseUAV;
		res = dxDevice->CreateUnorderedAccessView( valueNoiseBuff, &uavDesc, &valueNoiseUAV );
		if ( FAILED(res) )
		{
			safeRelease(&valueNoiseBuff);
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "NoiseRendererGPU::create2DValueNoise aborted: Error creating unordered access view" << std::endl;
			return seed;
		}


		// create and fill structured buffer with random uints
		std::vector<float> ranNums;
		ranNums.reserve( STRUCTBUFF_SIZE );
		for ( UINT i = 0; i < STRUCTBUFF_SIZE; ++i )
			ranNums.push_back( randGen() / float(randGen.max()) );
			
		ZeroMemory( &subResourceData, sizeof( subResourceData ) );
		subResourceData.pSysMem = ranNums.data();
		buffDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
		buffDesc.ByteWidth           = STRUCTBUFF_SIZE * sizeof(float);
		buffDesc.StructureByteStride = sizeof(float);
		ID3D11Buffer *ranNumBuff;
		res = dxDevice->CreateBuffer(&buffDesc, &subResourceData, &ranNumBuff);
		if ( FAILED(res) )
		{	
			safeRelease(&valueNoiseUAV);
			safeRelease(&valueNoiseBuff);
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "NoiseRendererGPU::create2DValueNoise aborted: Error creating structured buffer" << std::endl;
			return seed;
		}

		// create SRV for random float buffer
		ID3D11ShaderResourceView *randomNumSRV;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory( &srvDesc, sizeof(srvDesc) );
		srvDesc.Format        = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements  = STRUCTBUFF_SIZE;
		res = dxDevice->CreateShaderResourceView( ranNumBuff, &srvDesc, &randomNumSRV );
		if ( FAILED(res) )
		{
			safeRelease(&ranNumBuff);
			safeRelease(&valueNoiseUAV);
			safeRelease(&valueNoiseBuff);
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "NoiseRendererGPU::create2DValueNoise aborted: Error creating SRV for structured buffer" << std::endl;
			return seed;
		}


		// unload any possible other shaders form the pipeline
		dxDevContext->VSSetShader( NULL, NULL, 0 );
		dxDevContext->HSSetShader( NULL, NULL, 0 );
		dxDevContext->DSSetShader( NULL, NULL, 0 );
		dxDevContext->GSSetShader( NULL, NULL, 0 );
		dxDevContext->PSSetShader( NULL, NULL, 0 );

		UINT dispatchX = noiseOutputTexDesc.Width / 16;
		UINT dispatchY = noiseOutputTexDesc.Height / 16;

		float amplitude   = 1.0f;
		float totalAmp    = 0.0f;
		for ( int oct = octaves - 1; oct >= 0; --oct )
		{
			// update noise parameters
			amplitude *= persistance;
			totalAmp  += amplitude;

			// update smoothNoiseParams buffer
			int period = 1 << oct;
			float frequency = 1.0f / period;
			SmoothNoiseParams smNoiseParams;
			smNoiseParams.amplitude = amplitude;
			smNoiseParams.frequency = frequency;
			smNoiseParams.stretchY  = stretchY;
			smNoiseParams.period    = period;
			smNoiseParams.imgWidth  = noiseOutputTexDesc.Width;
			smNoiseParams.imgHeight = noiseOutputTexDesc.Height;

			D3D11_MAPPED_SUBRESOURCE mapRes;
			dxDevContext->Map(smoothNoiseParamsBuff, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapRes);
			memcpy( mapRes.pData, &smNoiseParams, sizeof(SmoothNoiseParams) );
			dxDevContext->Unmap(smoothNoiseParamsBuff, NULL);

			dxDevContext->CSSetShader( smoothNoiseShader, NULL, 0 );
			dxDevContext->CSSetShaderResources( 0, 1, &randomNumSRV );
			dxDevContext->CSSetUnorderedAccessViews( 0, 1, &valueNoiseUAV, NULL );
			dxDevContext->CSSetConstantBuffers( 0, 1, &smoothNoiseParamsBuff );
			//std::cout << "Dispatch( " << dispatchX << ", " << dispatchY << ", 1 );" << std::endl;
			dxDevContext->Dispatch( dispatchX, dispatchY, 1 );
		}


		// update noiseAverage constant buffer
		D3D11_MAPPED_SUBRESOURCE mapRes;
		NoiseAvgParams noiseAvgParams = { bias, gain, totalAmp, heightDiff };
		dxDevContext->Map(noiseAvgParamsBuff, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapRes);
		memcpy( mapRes.pData, &noiseAvgParams, sizeof(NoiseAvgParams) );
		dxDevContext->Unmap(noiseAvgParamsBuff, NULL);

		// load noiseAverage shader
		dxDevContext->CSSetShader( noiseAvgShader, NULL, 0 );
		ID3D11UnorderedAccessView *uavs[2] = { valueNoiseUAV, renderTexUAV };
		dxDevContext->CSSetUnorderedAccessViews( 0, 2, uavs, NULL );
		dxDevContext->CSSetConstantBuffers( 0, 1, &noiseAvgParamsBuff );

		//std::cout << "Dispatch( " << dispatchX << ", " << dispatchY << ", 1 );" << std::endl;
		dxDevContext->Dispatch( dispatchX, dispatchY, 1 );


		dxDevContext->CopyResource( noiseOutputTex, renderTex );

		// release temporary resources
		safeRelease(&randomNumSRV);
		safeRelease(&ranNumBuff);
		safeRelease(&valueNoiseUAV);
		safeRelease(&valueNoiseBuff);
		safeRelease(&renderTexUAV);
		safeRelease(&renderTex);

		return seed;
	}


	long NoiseRendererGPU::createMaturationFrameNoise( _Inout_ Ogre::D3D11Texture *texture, long s )
	{
		ID3D11Texture2D *outputTex = texture->GetTex2D();
		D3D11_TEXTURE2D_DESC outputTexDesc;
		outputTex->GetDesc( &outputTexDesc );

		// create render output texture
		D3D11_TEXTURE2D_DESC renTex2DDesc;
		ZeroMemory( &renTex2DDesc, sizeof(renTex2DDesc) );
		renTex2DDesc.Width              = outputTexDesc.Width;
		renTex2DDesc.Height             = outputTexDesc.Height;
		renTex2DDesc.MipLevels          = outputTexDesc.MipLevels;
		renTex2DDesc.ArraySize          = outputTexDesc.ArraySize;
		renTex2DDesc.SampleDesc.Count   = outputTexDesc.SampleDesc.Count;
		renTex2DDesc.SampleDesc.Quality = outputTexDesc.SampleDesc.Quality;
		renTex2DDesc.Usage              = D3D11_USAGE_DEFAULT;
		renTex2DDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
		renTex2DDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

		ID3D11Texture2D *renderTex;
		HRESULT res = dxDevice->CreateTexture2D( &renTex2DDesc, NULL, &renderTex );
		if ( FAILED(res) )
		{
			std::cout << "NoiseRendererGPU::detectEdges aborted: Error creating render texture" << std::endl;
			return -1;
		}

		// create render texture UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		uavDesc.Format             = renTex2DDesc.Format;
		uavDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		ID3D11UnorderedAccessView *renderTexUAV;
		res = dxDevice->CreateUnorderedAccessView( renderTex, &uavDesc, &renderTexUAV );
		if ( FAILED(res) )
		{
			safeRelease(&renderTex);
			std::cout << "NoiseRendererGPU::detectEdges aborted: Error creating UAV" << std::endl;
			return -1;
		}

		// unload any used shader resources
		ID3D11ShaderResourceView *noSrv[1] = {0};
		dxDevContext->CSSetShaderResources( 0, 1, noSrv );
		ID3D11UnorderedAccessView *noUav[1] = {0};
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, noUav, NULL );
		ID3D11Buffer *noConstBuff[1] = {0};
		dxDevContext->CSSetConstantBuffers( 0, 1, noConstBuff );

		long newSeed = create2DValueNoise( texture, 6, 0.21f, 0.34f, 0.28f, s );

		dxDevContext->CSSetShader( maturationNoiseShader, NULL, 0 );
		ID3D11ShaderResourceView *inTexSRV = texture->getTexture();
		dxDevContext->CSSetShaderResources( 0, 1, &inTexSRV );
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, &renderTexUAV, NULL );

		UINT dispatchX = outputTexDesc.Width / 16;
		UINT dispatchY = outputTexDesc.Height / 16;
		dxDevContext->Dispatch( dispatchX, dispatchY, 1 );

		// copy generated texture to output texture
		dxDevContext->CopyResource( outputTex, renderTex );

		// release temporary resources
		safeRelease(&renderTexUAV);
		safeRelease(&renderTex);

		return newSeed;
	}


	bool NoiseRendererGPU::setupShaders()
	{
		//load shader sources
		HRESULT res = E_FAIL;

		std::wstring files[3] = { L"SmoothNoiseShaderCS.cso", L"NoiseAverageCS.cso", L"MaturationNoiseCS.cso" };
		ID3D11ComputeShader **shaders[3] = { &smoothNoiseShader, &noiseAvgShader, &maturationNoiseShader };
		for ( int i = 0; i < 3; ++i )
		{
			ID3DBlob *shaderSource;
			res = D3DReadFileToBlob( files[i].c_str(), &shaderSource );
			if ( FAILED(res) )
			{
				std::cout << "NoiseRendererGPU::setupShaders aborted: Error loading shader source" << std::endl;
				//TODO: implement proper exception throwing
				return false;
			}
		
			res = dxDevice->CreateComputeShader( shaderSource->GetBufferPointer(), shaderSource->GetBufferSize(), NULL, shaders[i] );		
			shaderSource->Release();

			if ( FAILED(res) )
			{
				std::cout << "NoiseRendererGPU::setupShaders aborted: Error creating shader" << std::endl;
				return false;
			}
		}
		

		// create constant buffer for passing shader parameters
		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.ByteWidth      = ((sizeof(SmoothNoiseParams) - 1) / 16 + 1) * 16;		//ByteWidth has to be a multiple of 16 for constant buffers;
		buffDesc.Usage          = D3D11_USAGE_DYNAMIC;
		buffDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		// constant buffer for SmoothNoiseShader
		res = dxDevice->CreateBuffer( &buffDesc, NULL, &smoothNoiseParamsBuff );
		if ( FAILED(res) )
		{
			std::cout << "NoiseRendererGPU::setupShaders aborted: Error creating constant buffer" << std::endl;
			safeRelease(&noiseAvgShader);
			safeRelease(&smoothNoiseShader);
			return false;
		}

		// contant buffer for NoiseAmplitude and NoiseAverage shader
		buffDesc.ByteWidth = ((sizeof(NoiseAvgParams) - 1) / 16 + 1) * 16;		//ByteWidth has to be a multiple of 16 for constant buffers;
		res = dxDevice->CreateBuffer( &buffDesc, NULL, &noiseAvgParamsBuff );
		if ( FAILED(res) )
		{
			std::cout << "NoiseRendererGPU::setupShaders aborted: Error creating constant buffer" << std::endl;
			safeRelease(&smoothNoiseParamsBuff);
			safeRelease(&noiseAvgShader);
			safeRelease(&smoothNoiseShader);
			return false;
		}

		isInit = true;
		return true;
	}
}