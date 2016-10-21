
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};

Texture2D woundShapeUpperTex : register(t0);
Texture2D woundShapeLowerTex : register(t1);
Texture2D edgeTex            : register(t2);
Texture2D upperSkinTex       : register(t3);
Texture2D lowerSkinTex       : register(t4);

SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 edgeColor = edgeTex.Sample(samp, input.texCoord);
	edgeColor = float4(0.824f, 0.308f, 0.355f, edgeColor.a * 0.48f);

	// multiply colors
	float4 upperSkinColor  = upperSkinTex.Sample(samp, input.texCoord);
	float3 multiplyColor = edgeColor.rgb * upperSkinColor.rgb;

	if ( edgeColor.a <= 0.0001f )
	{
		float4 woundShapeLower = woundShapeLowerTex.Sample(samp, input.texCoord);	
		if ( woundShapeLower.a > 0.0001f )
		{
			float4 innerWoundClr = lowerSkinTex.Sample(samp, input.texCoord);
			return innerWoundClr * float4(0.785f, 0.0f, 0.0f, 1.0f);
		}
		
		float4 woundShapeUpper = woundShapeUpperTex.Sample(samp, input.texCoord);
		if ( woundShapeUpper.a > 0.0001f )
		{
			float4 edgeWoundClr = upperSkinTex.Sample(samp, input.texCoord);
			return edgeWoundClr * float4(0.95f, 0.95f, 0.95f, 1.0f);
		}

		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	
	return float4( multiplyColor, edgeColor.a );
}
