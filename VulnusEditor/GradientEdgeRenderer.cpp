
#include <chrono>
#include <d3dcompiler.h>

#include <OgreVector3.h>
#include <OgreVector4.h>

#include "GradientEdgeRenderer.h"


namespace Vulnus
{
	ID3D11Device* GradientEdgeRenderer::dxDevice                  = 0;
	ID3D11DeviceContext* GradientEdgeRenderer::dxDevContext       = 0;
	ID3D11ComputeShader* GradientEdgeRenderer::fadeShader         = 0;
	ID3D11Buffer *GradientEdgeRenderer::fadeShaderContantBuff     = 0;
	ID3D11Texture2D *GradientEdgeRenderer::renderTex              = 0;
	ID3D11UnorderedAccessView *GradientEdgeRenderer::renderTexUAV = 0;
	ID3D11Buffer *GradientEdgeRenderer::hullBuff                  = 0;
	ID3D11ShaderResourceView *GradientEdgeRenderer::hullSRV       = 0;
	bool GradientEdgeRenderer::isInit                             = false;
	bool GradientEdgeRenderer::fadeToTexture                      = false;


	void GradientEdgeRenderer::setupRenderer( _In_ ID3D11Device* dxDev )
	{
		dxDevice = dxDev;
		dxDevice->GetImmediateContext( &dxDevContext );
	}

	void GradientEdgeRenderer::cleanup()
	{
		safeRelease(&hullSRV);
		safeRelease(&hullBuff);
		safeRelease(&renderTexUAV);
		safeRelease(&renderTex);
		safeRelease(&fadeShaderContantBuff);
		safeRelease(&fadeShader);
		safeRelease(&dxDevContext);
	}


	void GradientEdgeRenderer::fadeTo( _Inout_ Ogre::D3D11Texture *outputTexture, _In_ Ogre::D3D11Texture *woundShapeTexture,
		_In_ const std::vector<Utils::HullPoint> *hull, RenderSide renderSide, const Ogre::Vector4 &fadeClr, UINT thickness )
	{
		fadeToTexture = true;
		fadeTo( outputTexture, woundShapeTexture, hull, renderSide, fadeClr, Ogre::Vector4(0.0f, 0.0f, 0.0f, 0.0f), thickness );
		fadeToTexture = false;
	}

	void GradientEdgeRenderer::fadeTo( _Inout_ Ogre::D3D11Texture *outputTexture, _In_ Ogre::D3D11Texture *woundShapeTexture,
									_In_ const std::vector<Utils::HullPoint> *hull, RenderSide renderSide, const Ogre::Vector4 &fromClr,
									const Ogre::Vector4 &toClr, UINT thickness )
	{
		if ( !isInit && !setupShaders() )
			return;

		setupForRendering( outputTexture, hull );

		// unload any possible other shaders form the pipeline
		dxDevContext->VSSetShader( NULL, NULL, 0 );
		dxDevContext->HSSetShader( NULL, NULL, 0 );
		dxDevContext->DSSetShader( NULL, NULL, 0 );
		dxDevContext->GSSetShader( NULL, NULL, 0 );
		dxDevContext->PSSetShader( NULL, NULL, 0 );

		ID3D11Texture2D *outputTex = outputTexture->GetTex2D();
		D3D11_TEXTURE2D_DESC outputTexDesc;
		outputTex->GetDesc( &outputTexDesc );
		UINT dispatchX = outputTexDesc.Width / 16;
		UINT dispatchY = outputTexDesc.Height / 16;


		// update shader constant buffer
		FadeShaderParams sParams;
		sParams.hullBuffSize  = UINT(hull->size());
		sParams.renderSide    = UINT(renderSide);
		sParams.thickness     = thickness;
		sParams.fadeToTexture = fadeToTexture;
		sParams.fromColor[0]  = fromClr.x;
		sParams.fromColor[1]  = fromClr.y;
		sParams.fromColor[2]  = fromClr.z;
		sParams.fromColor[3]  = fromClr.w;
		sParams.toColor[0]    = toClr.x;
		sParams.toColor[1]    = toClr.y;
		sParams.toColor[2]    = toClr.z;
		sParams.toColor[3]    = toClr.w;
		D3D11_MAPPED_SUBRESOURCE mapRes;
		dxDevContext->Map(fadeShaderContantBuff, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapRes);
		memcpy( mapRes.pData, &sParams, sizeof(FadeShaderParams) );
		dxDevContext->Unmap(fadeShaderContantBuff, NULL);


		// unbind resources
		ID3D11Buffer *noBuff[1] = {0};
		ID3D11UnorderedAccessView *noUav[1] = {0};	
		ID3D11ShaderResourceView *noSrv[1] = {0};
		
		//TODO: temporary test
		dxDevContext->CSSetConstantBuffers( 0, 1, noBuff );
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, noUav, NULL );
		dxDevContext->CSSetShaderResources( 0, 1, noSrv );

		dxDevContext->VSSetConstantBuffers( 0, 1, noBuff );
		dxDevContext->VSSetShaderResources( 0, 1, noSrv );

		dxDevContext->HSSetConstantBuffers( 0, 1, noBuff );
		dxDevContext->HSSetShaderResources( 0, 1, noSrv );

		dxDevContext->DSSetConstantBuffers( 0, 1, noBuff );
		dxDevContext->DSSetShaderResources( 0, 1, noSrv );

		dxDevContext->PSSetConstantBuffers( 0, 1, noBuff );
		dxDevContext->PSSetShaderResources( 0, 1, noSrv );

		dxDevContext->OMSetRenderTargetsAndUnorderedAccessViews(D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL, NULL, NULL, 0, 1, noUav, NULL );


		// setup shader
		dxDevContext->CSSetShader( fadeShader, NULL, 0 );
		dxDevContext->CSSetConstantBuffers( 0, 1, &fadeShaderContantBuff );
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, &renderTexUAV, NULL );

		ID3D11ShaderResourceView *srvs[3] = { hullSRV, outputTexture->getTexture(), woundShapeTexture->getTexture() };
		dxDevContext->CSSetShaderResources( 0, 3, srvs );

		//std::cout << "Dispatch( " << dispatchX << ", " << dispatchY << ", 1 );" << std::endl;
		dxDevContext->Dispatch( dispatchX, dispatchY, 1 );

		dxDevContext->CopyResource( outputTex, renderTex );


		// unbind resources
		dxDevContext->CSSetConstantBuffers( 0, 1, noBuff );
		dxDevContext->CSSetUnorderedAccessViews( 0, 1, noUav, NULL );
		dxDevContext->CSSetShaderResources( 0, 1, noSrv );

		// release temporary resources
		safeRelease(&hullSRV);
		safeRelease(&hullBuff);
		safeRelease(&renderTexUAV);
		safeRelease(&renderTex);
	}


	bool GradientEdgeRenderer::setupShaders()
	{
		//load shader sources
		ID3DBlob *shaderSource = NULL;
		HRESULT res = D3DReadFileToBlob( L"GradientEdgeCS.cso", &shaderSource );
		if ( FAILED(res) )
		{
			std::cout << "GradientEdgeRenderer::setupShaders aborted: Error loading shader source" << std::endl;
			//TODO: implement proper exception throwing
			return false;
		}
		
		res = dxDevice->CreateComputeShader( shaderSource->GetBufferPointer(), shaderSource->GetBufferSize(), NULL, &fadeShader );		
		shaderSource->Release();

		if ( FAILED(res) )
		{
			std::cout << "GradientEdgeRenderer::setupShaders aborted: Error creating shader" << std::endl;
			return false;
		}


		// create constant buffers for passing shader parameters
		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.ByteWidth      = ((sizeof(FadeShaderParams) - 1) / 16 + 1) * 16;		//ByteWidth has to be a multiple of 16 for constant buffers;
		buffDesc.Usage          = D3D11_USAGE_DYNAMIC;
		buffDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		// constant buffer for fadeShader
		res = dxDevice->CreateBuffer( &buffDesc, NULL, &fadeShaderContantBuff );
		if ( FAILED(res) )
		{
			std::cout << "GradientEdgeRenderer::setupShaders aborted: Error creating constant buffer" << std::endl;
			safeRelease(&fadeShader);
			return false;
		}

		isInit = true;
		return true;
	}


	void GradientEdgeRenderer::setupForRendering( _In_ Ogre::D3D11Texture *inputTexture, _In_ const std::vector<Utils::HullPoint> *hull )
	{
		ID3D11Texture2D *inputTex = inputTexture->GetTex2D();
		D3D11_TEXTURE2D_DESC inputTexDesc;
		inputTex->GetDesc( &inputTexDesc );

		// create render output texture
		D3D11_TEXTURE2D_DESC renTex2DDesc;
		ZeroMemory( &renTex2DDesc, sizeof(renTex2DDesc) );
		renTex2DDesc.Width              = inputTexDesc.Width;
		renTex2DDesc.Height             = inputTexDesc.Height;
		renTex2DDesc.MipLevels          = inputTexDesc.MipLevels;
		renTex2DDesc.ArraySize          = inputTexDesc.ArraySize;
		renTex2DDesc.SampleDesc.Count   = inputTexDesc.SampleDesc.Count;
		renTex2DDesc.SampleDesc.Quality = inputTexDesc.SampleDesc.Quality;
		renTex2DDesc.Usage              = D3D11_USAGE_DEFAULT;
		renTex2DDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
		renTex2DDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;

		HRESULT res = dxDevice->CreateTexture2D( &renTex2DDesc, NULL, &renderTex );
		if ( FAILED(res) )
		{
			std::cout << "GradientEdgeRenderer::setupForRendering aborted: Error creating render texture" << std::endl;
			return;
		}

		// create render texture UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		uavDesc.Format             = renTex2DDesc.Format;
		uavDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		res = dxDevice->CreateUnorderedAccessView( renderTex, &uavDesc, &renderTexUAV );
		if ( FAILED(res) )
		{
			safeRelease(&renderTex);
			std::cout << "GradientEdgeRenderer::setupForRendering aborted: Error creating unordered access view for render texture" << std::endl;
			return;
		}

		
		// create and fill structured buffer with hull points
		D3D11_SUBRESOURCE_DATA subResourceData;
		ZeroMemory( &subResourceData, sizeof(subResourceData) );
		subResourceData.pSysMem = hull->data();

		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
		buffDesc.ByteWidth           = UINT(hull->size()) * sizeof(Utils::HullPoint);
		buffDesc.StructureByteStride = sizeof(Utils::HullPoint);
		buffDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		res = dxDevice->CreateBuffer(&buffDesc, &subResourceData, &hullBuff);
		if ( FAILED(res) )
		{	
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "GradientEdgeRenderer::setupForRendering aborted: Error creating structured buffer" << std::endl;
			return;
		}

		// create SRV for hull points buffer
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory( &srvDesc, sizeof(srvDesc) );
		srvDesc.Format        = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements  = UINT(hull->size());
		res = dxDevice->CreateShaderResourceView( hullBuff, &srvDesc, &hullSRV );
		if ( FAILED(res) )
		{
			safeRelease(&hullBuff);
			safeRelease(&renderTexUAV);
			safeRelease(&renderTex);
			std::cout << "GradientEdgeRenderer::setupForRendering aborted: Error creating SRV for structured buffer" << std::endl;
			return;
		}
	}
}
