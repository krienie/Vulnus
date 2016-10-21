
#include <iostream>
#include <limits>
#include <d3dcompiler.h>

#include "Renderer.h"
#include "WoundErosionFilter.h"

namespace Vulnus
{
	WoundErosionFilter::WoundErosionFilter( _In_ ID3D11Device* dxDev )
		: dxDevice(dxDev), dxDevContext(0), erodeShader(0), isInit(false)
	{
		dxDevice->GetImmediateContext( &dxDevContext );
	}

	WoundErosionFilter::~WoundErosionFilter()
	{
		safeRelease(&erodeShader);
		safeRelease(&dxDevContext);
	}


	void WoundErosionFilter::erode( _Inout_ Ogre::D3D11Texture *tex, UINT kernWidth )
	{
		if ( !isInit && !setupKernel() )
			return;

		ID3D11Texture2D *convTex = tex->GetTex2D();
		D3D11_TEXTURE2D_DESC convTexDesc;
		convTex->GetDesc( &convTexDesc );

		// create render output texture
		D3D11_TEXTURE2D_DESC renTex2DDesc;
		ZeroMemory( &renTex2DDesc, sizeof(renTex2DDesc) );
		renTex2DDesc.Width              = convTexDesc.Width;
		renTex2DDesc.Height             = convTexDesc.Height;
		renTex2DDesc.MipLevels          = convTexDesc.MipLevels;
		renTex2DDesc.ArraySize          = convTexDesc.ArraySize;
		renTex2DDesc.SampleDesc.Count   = convTexDesc.SampleDesc.Count;
		renTex2DDesc.SampleDesc.Quality = convTexDesc.SampleDesc.Quality;
		renTex2DDesc.Usage              = D3D11_USAGE_DEFAULT;
		renTex2DDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
		renTex2DDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

		ID3D11Texture2D *renderTex;
		HRESULT res = dxDevice->CreateTexture2D( &renTex2DDesc, NULL, &renderTex );

		if ( FAILED(res) )
		{
			std::cout << "WoundErosionFilter::erode aborted: Error creating render texture" << std::endl;
			return;
		}

		// create render texture UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC renderTexUAVDesc;
		ZeroMemory( &renderTexUAVDesc, sizeof(renderTexUAVDesc) );
		renderTexUAVDesc.Format             = renTex2DDesc.Format;
		renderTexUAVDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		renderTexUAVDesc.Texture2D.MipSlice = 0;
		ID3D11UnorderedAccessView *renderTexUAV;
		res = dxDevice->CreateUnorderedAccessView( renderTex, &renderTexUAVDesc, &renderTexUAV );

		if ( FAILED(res) )
		{
			safeRelease(&renderTex);
			std::cout << "WoundErosionFilter::erode aborted: Error creating unordered access view for render texture" << std::endl;
			return;
		}

		// create and fill structured buffer with random uints
		const UINT RANNUMS_SIZE = convTexDesc.Width * convTexDesc.Height;
		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
		buffDesc.ByteWidth           = RANNUMS_SIZE * sizeof(UINT);
		buffDesc.StructureByteStride = sizeof(UINT);
		buffDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		buffDesc.Usage               = D3D11_USAGE_DEFAULT;
		
		std::vector<UINT> ranNums;
		ranNums.reserve( RANNUMS_SIZE );
		for ( UINT i = 0; i < RANNUMS_SIZE; ++i )
			ranNums.push_back( rand() % std::max(kernWidth - 3, 3U) + 3 );		// kernel width varying between 3 and kernWidth, inclusive
			
		D3D11_SUBRESOURCE_DATA subResourceData;
		ZeroMemory( &subResourceData, sizeof( subResourceData ) );
		subResourceData.pSysMem = ranNums.data();

		ID3D11Buffer *ranNumBuff;
		res = dxDevice->CreateBuffer(&buffDesc, &subResourceData, &ranNumBuff);
		if ( FAILED(res) )
		{
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);	
			std::cout << "WoundErosionFilter::erode aborted: Error creating structured buffer" << std::endl;
			return;
		}

		// create SRV for random uint buffer
		ID3D11ShaderResourceView *randomNumSRV;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory( &srvDesc, sizeof(srvDesc) );
		srvDesc.Format        = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements  = RANNUMS_SIZE;
		res = dxDevice->CreateShaderResourceView( ranNumBuff, &srvDesc, &randomNumSRV );
		if ( FAILED(res) )
		{
			safeRelease(&ranNumBuff);
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "WoundErosionFilter::erode aborted: Error creating SRV for structured buffer" << std::endl;
			return;
		}


		// unload any possible other shaders form the pipeline
		dxDevContext->VSSetShader( NULL, NULL, 0 );
		dxDevContext->HSSetShader( NULL, NULL, 0 );
		dxDevContext->DSSetShader( NULL, NULL, 0 );
		dxDevContext->GSSetShader( NULL, NULL, 0 );
		dxDevContext->PSSetShader( NULL, NULL, 0 );

		ID3D11ShaderResourceView *inTexSRV = tex->getTexture();
		dxDevContext->CSSetShader( erodeShader, NULL, 0 );

		ID3D11ShaderResourceView *srvs[2] = { randomNumSRV, inTexSRV };
		dxDevContext->CSSetShaderResources( 0, 2, srvs );
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, &renderTexUAV, NULL );

		UINT x = convTexDesc.Width / 16;
		UINT y = convTexDesc.Height / 16;

		//std::cout << "Dispatch( " << x << ", " << y << ", 1 );" << std::endl;

		dxDevContext->Dispatch( x, y, 1 );

		ID3D11Texture2D *outTex = tex->GetTex2D();
		dxDevContext->CopyResource( outTex, renderTex );


		// release temporary resources
		safeRelease(&randomNumSRV);
		safeRelease(&ranNumBuff);
		safeRelease(&renderTexUAV);
		safeRelease(&renderTex);
	}


	bool WoundErosionFilter::setupKernel()
	{
		//load shader sources
		HRESULT res = E_FAIL;

		ID3DBlob *shaderSource;
		res = D3DReadFileToBlob( L"WoundErosionKernelCS.cso", &shaderSource );
		if ( FAILED(res) )
		{
			std::cout << "WoundErosionFilter::setupKernel aborted: Error loading shader source" << std::endl;
			//TODO: implement proper exception throwing
			return false;
		}
		
		res = dxDevice->CreateComputeShader( shaderSource->GetBufferPointer(), shaderSource->GetBufferSize(), NULL, &erodeShader );		
		shaderSource->Release();

		if ( FAILED(res) )
		{
			std::cout << "WoundErosionFilter::setupKernel aborted: Error creating shader" << std::endl;
			return false;
		}

		isInit = true;
		return true;
	}
}
