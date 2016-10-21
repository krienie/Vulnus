
#include <chrono>
#include <d3dcompiler.h>

#include "NormalmapGen.h"


namespace Vulnus
{
	ID3D11Device* NormalmapGen::dxDevice                    = 0;
	ID3D11DeviceContext* NormalmapGen::dxDevContext         = 0;
	ID3D11ComputeShader* NormalmapGen::heightToNormalShader = 0;
	bool NormalmapGen::isInit                               = false;


	void NormalmapGen::setupRenderer( _In_ ID3D11Device* dxDev )
	{
		dxDevice = dxDev;
		dxDevice->GetImmediateContext( &dxDevContext );
	}

	void NormalmapGen::cleanup()
	{
		safeRelease(&heightToNormalShader);
		safeRelease(&dxDevContext);
	}


	void NormalmapGen::fromHeightmap( _Inout_ Ogre::D3D11Texture *texture )
	{
		if ( !isInit && !setupShaders() )
			return;

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
			std::cout << "NormalmapGen::fromHeightmap aborted: Error creating render texture" << std::endl;
			return;
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
			std::cout << "NormalmapGen::fromHeightmap aborted: Error creating unordered access view for render texture" << std::endl;
			return;
		}


		// unload any possible other shaders form the pipeline
		dxDevContext->VSSetShader( NULL, NULL, 0 );
		dxDevContext->HSSetShader( NULL, NULL, 0 );
		dxDevContext->DSSetShader( NULL, NULL, 0 );
		dxDevContext->GSSetShader( NULL, NULL, 0 );
		dxDevContext->PSSetShader( NULL, NULL, 0 );

		UINT dispatchX = outputTexDesc.Width / 16;
		UINT dispatchY = outputTexDesc.Height / 16;

		// load noiseAverage shader
		dxDevContext->CSSetShader( heightToNormalShader, NULL, 0 );
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, &renderTexUAV, NULL );
		ID3D11ShaderResourceView *srv = texture->getTexture();
		dxDevContext->CSSetShaderResources( 0, 1, &srv );

		//std::cout << "Dispatch( " << dispatchX << ", " << dispatchY << ", 1 );" << std::endl;
		dxDevContext->Dispatch( dispatchX, dispatchY, 1 );


		dxDevContext->CopyResource( outputTex, renderTex );

		// release temporary resources
		safeRelease(&renderTexUAV);
		safeRelease(&renderTex);
	}


	bool NormalmapGen::setupShaders()
	{
		//load shader sources
		ID3DBlob *shaderSource = NULL;
		HRESULT res = D3DReadFileToBlob( L"HeightmapToNormalCS.cso", &shaderSource );
		if ( FAILED(res) )
		{
			std::cout << "NormalmapGen::setupShaders aborted: Error loading shader source" << std::endl;
			//TODO: implement proper exception throwing
			return false;
		}
		
		res = dxDevice->CreateComputeShader( shaderSource->GetBufferPointer(), shaderSource->GetBufferSize(), NULL, &heightToNormalShader );		
		shaderSource->Release();

		if ( FAILED(res) )
		{
			std::cout << "NormalmapGen::setupShaders aborted: Error creating shader" << std::endl;
			return false;
		}

		isInit = true;
		return true;
	}
}