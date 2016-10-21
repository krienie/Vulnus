
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>

#include "Renderer.h"
#include "GradientEdgeRenderer.h"
#include "NoiseRendererGPU.h"
#include "SharedResources.h"
#include "MaturationFrame.h"

namespace Vulnus
{
	MaturationFrame::MaturationFrame( _In_ Ogre::Camera *camera, _In_ Ogre::Entity *model, _In_ ID3D11Device* dxDev )
		: Keyframe("MaturationFrame", "maturationFrameAlbedo", "maturationFrameNormal",
						"maturationFrameReflection", "maturationFrameHeight", camera, model),
			noiseSeed( NoiseRendererGPU::getNewSeed() ), closingDistance(0.0f), resourcesSetup(false), dxDevice(dxDev),
			dxDevContext(0), gradientShader(0), gradientShaderContantBuff(0), reflectionNoiseTex(0), reflectionNoiseMaskTex(0),
			albedoRenderTex(0), normalRenderTex(0), reflectRenderTex(0), albedoRenderTexUAV(0), normalRenderTexUAV(0), reflectRenderTexUAV(0)
	{
		dxDevice->GetImmediateContext(&dxDevContext);

		reflectionNoiseTex     = dynamic_cast<Ogre::D3D11Texture*>( Renderer::getManualTexture( "MaturationFrameReflectionNoiseTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE ) );
		reflectionNoiseMaskTex = dynamic_cast<Ogre::D3D11Texture*>( Renderer::getManualTexture( "MaturationFrameReflectionNoiseMaskTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE ) );
	}

	MaturationFrame::~MaturationFrame()
	{
		safeRelease(&gradientShaderContantBuff);
		safeRelease(&gradientShader);
		safeRelease(&albedoRenderTexUAV);
		safeRelease(&albedoRenderTex);
		safeRelease(&normalRenderTexUAV);
		safeRelease(&normalRenderTex);
		safeRelease(&reflectRenderTexUAV);
		safeRelease(&reflectRenderTex);
		safeRelease(&dxDevContext);
	}


	MultiRTTSampler::SampledTexture MaturationFrame::renderFrame()
	{
		clock_t tStart = clock();
		std::cout << "starting rendering MaturationFrame" << std::endl;

		recalculateClosingDistance();

		Ogre::Texture *innerNoiseTex = Renderer::getManualTexture("maturationPhaseInnerNoiseTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		NoiseRendererGPU::create2DValueNoise( dynamic_cast<Ogre::D3D11Texture*>(innerNoiseTex), 5, 0.34f, 0.10f, 0.10f, noiseSeed );

		// setup heightmap renderpass
		Ogre::Pass *renderPass            = Ogre::MaterialManager::getSingleton().getByName( heightMatName )->getTechnique(0)->getPass(0);
		Ogre::Texture *woundShapeUpperTex = SharedResources::getTexture( SharedResources::WoundShapeUpperSkin );
		Ogre::TextureUnitState *texState  = renderPass->getTextureUnitState( woundShapeUpperTex->getName() );
		if ( !texState )
		{
			texState = renderPass->createTextureUnitState( woundShapeUpperTex->getName() );
			texState->setName( woundShapeUpperTex->getName() );
		}

		// only render height => albedo, normal and reflection are calculated in sample()
		renderTexture(TextureType::Height);

		// calculate reflection noise texture
		NoiseRendererGPU::createMaturationFrameNoise( reflectionNoiseTex, noiseSeed );
		NoiseRendererGPU::create2DValueNoise( reflectionNoiseMaskTex, 6, 0.64f, 0.20f, 0.79f, noiseSeed );


		SampledTexture output;
		output.albedo     = albedoTex.dataTex;
		output.normal     = normalTex.dataTex;
		output.reflection = reflectionTex.dataTex;
		output.height     = heightTex.dataTex;

		double time = double( clock() - tStart ) / (double)CLOCKS_PER_SEC;
		std::cout << "done in: " << time << " seconds" << std::endl;

		return output;
	}


	MultiRTTSampler::SampledTexture MaturationFrame::sample( double position )
	{
		// only render if something has been drawn
		if ( SharedResources::patchEdgeListPtr != 0 && SharedResources::patchEdgeListPtr->size() >= 1 )
			generateWoundColor( position );

		SampledTexture output;
		output.albedo     = albedoTex.dataTex;
		output.normal     = normalTex.dataTex;
		output.reflection = reflectionTex.dataTex;
		output.height     = heightTex.dataTex;

		return output;
	}


	void MaturationFrame::recalculateClosingDistance()
	{
		float maxDist = 0.0f;
		const std::vector<Utils::HullPoint> *hull = SharedResources::patchEdgeListPtr;
		std::vector<Utils::HullPoint>::const_iterator hullIt;
		for ( hullIt = hull->cbegin(); hullIt != hull->cend(); ++hullIt )
		{
			std::vector<Utils::HullPoint>::const_iterator pIt;
			for ( pIt = hull->cbegin(); pIt != hull->cend(); ++pIt )
			{
				Utils::HullPoint distVect = Utils::HullPoint( pIt->u - hullIt->u, pIt->v - hullIt->v );
				float curDist = std::sqrtf( std::powf(distVect.u, 2.0f) + std::powf(distVect.v, 2.0f) );
				maxDist = std::max( curDist, maxDist );
			}
		}

		closingDistance = maxDist;
	}

	void MaturationFrame::generateWoundColor( double position )
	{
		if ( !resourcesSetup && !setupRenderResources() )
			return;


		// create and fill structured buffer with hull points
		D3D11_SUBRESOURCE_DATA subResourceData;
		ZeroMemory( &subResourceData, sizeof(subResourceData) );
		subResourceData.pSysMem = SharedResources::patchEdgeListPtr->data();

		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE;
		buffDesc.ByteWidth           = UINT(SharedResources::patchEdgeListPtr->size()) * sizeof(Utils::HullPoint);
		buffDesc.StructureByteStride = sizeof(Utils::HullPoint);
		buffDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		ID3D11Buffer *hullBuff = NULL;
		HRESULT res = dxDevice->CreateBuffer(&buffDesc, &subResourceData, &hullBuff);
		if ( FAILED(res) )
		{	
			std::cout << "MaturationFrame::setupForRendering aborted: Error creating structured buffer" << std::endl;
			return;
		}

		// create SRV for hull points buffer
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory( &srvDesc, sizeof(srvDesc) );
		srvDesc.Format        = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements  = UINT(SharedResources::patchEdgeListPtr->size());
		ID3D11ShaderResourceView *hullSRV = NULL;
		res = dxDevice->CreateShaderResourceView( hullBuff, &srvDesc, &hullSRV );
		if ( FAILED(res) )
		{
			safeRelease(&hullBuff);
			std::cout << "MaturationFrame::setupForRendering aborted: Error creating SRV for structured buffer" << std::endl;
			return;
		}


		// unload any possible other shaders form the pipeline
		dxDevContext->VSSetShader( NULL, NULL, 0 );
		dxDevContext->HSSetShader( NULL, NULL, 0 );
		dxDevContext->DSSetShader( NULL, NULL, 0 );
		dxDevContext->GSSetShader( NULL, NULL, 0 );
		dxDevContext->PSSetShader( NULL, NULL, 0 );

		
		// calculate inner- mid- and outerWoundBorder
		ID3D11Texture2D *d3dAlbedoTex = dynamic_cast<Ogre::D3D11Texture*>( albedoTex.dataTex )->GetTex2D();
		D3D11_TEXTURE2D_DESC renTexDesc;
		d3dAlbedoTex->GetDesc( &renTexDesc );

		// closingDistance is in UV coordinates
		float totalPhaseThickeness = closingDistance * 0.5f;
		// wound size is 40-60% after scab falls off
		//TODO: add randomness of 40-60%
		float totalGradThickness = totalPhaseThickeness * 0.40f;

		//float borderPos = static_cast<float>(position) * totalPhaseThickeness;
		float outerWoundBorder = totalPhaseThickeness * position;
		float innerWoundborder = outerWoundBorder + totalGradThickness;
		//std::cout << "borderDifference: " << (innerWoundborder - outerWoundBorder) << std::endl;
		//std::cout << "----------------------------" << std::endl;

		// update shader constant buffer
		GradientShaderParams sParams;
		sParams.phaseThickness   = totalPhaseThickeness;
		sParams.outerWoundBorder = totalPhaseThickeness * position;
		sParams.innerWoundBorder = sParams.outerWoundBorder + totalGradThickness;
		sParams.hullBuffSize = UINT(SharedResources::patchEdgeListPtr->size());
		D3D11_MAPPED_SUBRESOURCE mapRes;
		dxDevContext->Map(gradientShaderContantBuff, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapRes);
		memcpy( mapRes.pData, &sParams, sizeof(GradientShaderParams) );
		dxDevContext->Unmap(gradientShaderContantBuff, NULL);

		// setup shader
		dxDevContext->CSSetShader( gradientShader, NULL, 0 );
		dxDevContext->CSSetConstantBuffers( 0, 1, &gradientShaderContantBuff );
		ID3D11UnorderedAccessView *uavs[3] = { albedoRenderTexUAV, normalRenderTexUAV, reflectRenderTexUAV };
		dxDevContext->CSSetUnorderedAccessViews( 0, 3, uavs, NULL );

		Ogre::D3D11Texture *skinClrTex     = dynamic_cast<Ogre::D3D11Texture*>( SharedResources::getTexture(SharedResources::UpperSkinColor) );
		Ogre::D3D11Texture *handReflectTex = dynamic_cast<Ogre::D3D11Texture*>( Renderer::getManualTexture("Hand_Spec.tif", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE) );
		Ogre::D3D11Texture *handNormalTex  = dynamic_cast<Ogre::D3D11Texture*>( Renderer::getManualTexture("Hand_NM.tif", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE) );
		Ogre::D3D11Texture *woundShapeTex  = dynamic_cast<Ogre::D3D11Texture*>( SharedResources::getTexture(SharedResources::WoundShapeUpperSkin) );
		Ogre::D3D11Texture *innerNoiseTex  = dynamic_cast<Ogre::D3D11Texture*>( Renderer::getManualTexture("maturationPhaseInnerNoiseTex", Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE) );
		//TODO: remove the need for woundShapeTex by using technique used in WoundShapePainter to find if a pixel is inside or outside of the wound
		ID3D11ShaderResourceView *srvs[8] = { hullSRV, skinClrTex->getTexture(), handReflectTex->getTexture(), handNormalTex->getTexture(),
												woundShapeTex->getTexture(), reflectionNoiseTex->getTexture(), reflectionNoiseMaskTex->getTexture(),
												innerNoiseTex->getTexture() };
		dxDevContext->CSSetShaderResources( 0, 8, srvs );

		// dispatch shader execute command
		UINT dispatchX = renTexDesc.Width / 16;
		UINT dispatchY = renTexDesc.Height / 16;
		dxDevContext->Dispatch( dispatchX, dispatchY, 1 );

		// copy results to output textures
		dxDevContext->CopyResource( d3dAlbedoTex, albedoRenderTex );
		Ogre::D3D11Texture *d3dNormalTex = dynamic_cast<Ogre::D3D11Texture*>( normalTex.dataTex );
		dxDevContext->CopyResource( d3dNormalTex->GetTex2D(), normalRenderTex );
		Ogre::D3D11Texture *d3dReflectTex = dynamic_cast<Ogre::D3D11Texture*>( reflectionTex.dataTex );
		dxDevContext->CopyResource( d3dReflectTex->GetTex2D(), reflectRenderTex );

		// release temporary resources
		safeRelease(&hullSRV);
		safeRelease(&hullBuff);
	}

	void MaturationFrame::createRenderTexture( _Inout_ ID3D11Texture2D **renderTex, _Inout_ ID3D11UnorderedAccessView **renderTexUAV )
	{
		// get texture descriptor

		Ogre::D3D11Texture *frameTex = dynamic_cast<Ogre::D3D11Texture*>( albedoTex.dataTex );
		ID3D11Texture2D *d3dframeTex = frameTex->GetTex2D();
		D3D11_TEXTURE2D_DESC renTexDesc;
		d3dframeTex->GetDesc( &renTexDesc );

		// create render output texture
		D3D11_TEXTURE2D_DESC renTex2DDesc;
		ZeroMemory( &renTex2DDesc, sizeof(renTex2DDesc) );
		renTex2DDesc.Width              = renTexDesc.Width;
		renTex2DDesc.Height             = renTexDesc.Height;
		renTex2DDesc.MipLevels          = renTexDesc.MipLevels;
		renTex2DDesc.ArraySize          = renTexDesc.ArraySize;
		renTex2DDesc.SampleDesc.Count   = renTexDesc.SampleDesc.Count;
		renTex2DDesc.SampleDesc.Quality = renTexDesc.SampleDesc.Quality;
		renTex2DDesc.Usage              = D3D11_USAGE_DEFAULT;
		renTex2DDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
		renTex2DDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
		HRESULT res = dxDevice->CreateTexture2D( &renTex2DDesc, NULL, renderTex );
		if ( FAILED(res) )
		{
			std::cout << "MaturationFrame::createRenderTexture aborted: Error creating render texture" << std::endl;
			return;
		}

		// create render texture UAV
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory( &uavDesc, sizeof(uavDesc) );
		uavDesc.Format             = renTex2DDesc.Format;
		uavDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		res = dxDevice->CreateUnorderedAccessView( *renderTex, &uavDesc, renderTexUAV );
		if ( FAILED(res) )
		{
			safeRelease(renderTex);
			std::cout << "MaturationFrame::createRenderTexture aborted: Error creating unordered access view for render texture" << std::endl;
			return;
		}
	}

	bool MaturationFrame::setupRenderResources()
	{
		//load shader sources
		ID3DBlob *shaderSource = NULL;
		HRESULT res = D3DReadFileToBlob( L"MaturationFrameGradientEdgeCS.cso", &shaderSource );
		if ( FAILED(res) )
		{
			std::cout << "MaturationFrame::setupShaders aborted: Error loading shader source" << std::endl;
			return false;
		}
		
		res = dxDevice->CreateComputeShader( shaderSource->GetBufferPointer(), shaderSource->GetBufferSize(), NULL, &gradientShader );		
		shaderSource->Release();
		if ( FAILED(res) )
		{
			std::cout << "MaturationFrame::setupShaders aborted: Error creating shader" << std::endl;
			return false;
		}

		// create constant buffers for passing shader parameters
		D3D11_BUFFER_DESC buffDesc;
		ZeroMemory( &buffDesc, sizeof(buffDesc) );
		buffDesc.ByteWidth      = ((sizeof(GradientShaderParams) - 1) / 16 + 1) * 16;		//ByteWidth has to be a multiple of 16 for constant buffers;
		buffDesc.Usage          = D3D11_USAGE_DYNAMIC;
		buffDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
		buffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		// constant buffer for gradientShader
		res = dxDevice->CreateBuffer( &buffDesc, NULL, &gradientShaderContantBuff );
		if ( FAILED(res) )
		{
			std::cout << "MaturationFrame::setupShaders aborted: Error creating constant buffer" << std::endl;
			safeRelease(&gradientShader);
			return false;
		}


		// create render textures
		createRenderTexture( &albedoRenderTex, &albedoRenderTexUAV );
		createRenderTexture( &normalRenderTex, &normalRenderTexUAV );
		createRenderTexture( &reflectRenderTex, &reflectRenderTexUAV );

		resourcesSetup = true;
		return true;
	}
}
