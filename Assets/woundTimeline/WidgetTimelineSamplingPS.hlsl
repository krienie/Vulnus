
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


cbuffer tracerParams : register(b0)
{
	int numPhases;
	int forceInsideBlend;
};


Texture2D upperSkinTex   : register(t0);
Texture2D blendMaskTex   : register(t1);
Texture2DArray phaseTexs : register(t2);
SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 blendMask = blendMaskTex.Sample( samp, input.texCoord );

	float4 totalPatchColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	if ( !forceInsideBlend && blendMask.r < 0.001f )
	{
		// outside wound
		[unroll(8)]
		for ( int i = 0; i < numPhases; ++i )
		{
			float4 phaseTexClr = phaseTexs.Sample( samp, float3(input.texCoord, i) );
			totalPatchColor += float4( phaseTexClr.rgb * phaseTexClr.a, phaseTexClr.a );
		}

		float4 skinColor  = upperSkinTex.Sample(samp, input.texCoord);
		float4 finalColor = float4( totalPatchColor.rgb + (skinColor.rgb * (1.0f - totalPatchColor.a)), 1.0f );

		return finalColor;
	} else
	{
		// inside wound
		[unroll(8)] for ( int i = 0; i < numPhases; ++i )
		{
			float4 phaseTexClr = phaseTexs.Sample( samp, float3(input.texCoord, i) );
			totalPatchColor += phaseTexClr;
		}

		totalPatchColor /= numPhases;
		return totalPatchColor;
	}
}
