
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


cbuffer tracerParams : register(b0)
{
	float4 darkScabClr;
	float4 lightScabClr;
};


Texture2D woundShapeTex            : register(t0);
Texture2D edgeColorGradientTex     : register(t1);
Texture2D edgeColorGradientMaskTex : register(t2);
Texture2D scabInsideHeightTex      : register(t3);
SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 woundShapeUpper = woundShapeTex.Sample(samp, input.texCoord);
	if ( woundShapeUpper.a > 0.0001f )
	{
		float4 heightClr = scabInsideHeightTex.Sample( samp, input.texCoord );

		//lowerskinColor = float4( (fleshColor.rgb * fleshColor.a) + (lowerskinColor.rgb * (1.0f - fleshColor.a)), 1.0f );

		float4 edgeColorGradient = edgeColorGradientTex.Sample( samp, input.texCoord ) * edgeColorGradientMaskTex.Sample( samp, input.texCoord );
		//edgeColorGradient *= float4( float3(1.0f, 1.0f, 1.0f) - heightClr.rgb, 1.0f );

		heightClr = lerp( lightScabClr, darkScabClr, heightClr );
		//middleColor *= float4( float3(1.0f, 1.0f, 1.0f) - heightClr.rgb, 1.0f );
		
		float4 finalColor = float4( heightClr.rgb + (edgeColorGradient.rgb * (float3(1.0f, 1.0f, 1.0f) - heightClr.rgb)), 1.0f );
		//float4 finalColor = float4( edgeColorGradient.rgb + (middleColor.rgb * (float3(1.0f, 1.0f, 1.0f) - edgeColorGradient.rgb)), 1.0f );
		//float4 finalColor = saturate(middleColor + edgeColorGradient);

		return finalColor;
	}

	return float4(0.0f, 0.0f, 0.0f, 0.0f);
}