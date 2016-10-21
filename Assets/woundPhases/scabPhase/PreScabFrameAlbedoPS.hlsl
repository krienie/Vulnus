
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};

Texture2D woundShapeUpperTex : register(t0);
Texture2D woundShapeLowerTex : register(t1);
Texture2D specleNoiseTex     : register(t2);
Texture2D lowerNoiseTex      : register(t3);
Texture2D upperSkinTex       : register(t4);
Texture2D lowerSkinTex       : register(t5);

SamplerState samp : register(s0);



float4 main( PixelInput input ) : SV_TARGET
{
	float4 woundShapeLower = woundShapeLowerTex.Sample(samp, input.texCoord);	
	if ( woundShapeLower.a > 0.0001f )
	{
		float4 lowerNoiseClr  = lowerNoiseTex.Sample( samp, input.texCoord );
		float4 upperSkinColor = upperSkinTex.Sample(samp, input.texCoord);
		float4 lowerSkinColor = lowerSkinTex.Sample(samp, input.texCoord) * float4( 1.0f, 0.7070f, 0.7968f, 1.0f );
		float4 lowerWoundClr  = float4( (lowerSkinColor.rgb * lowerNoiseClr.r) + (upperSkinColor.rgb * (1.0f - lowerNoiseClr.r)), 1.0f );

		float4 specleNoiseClr = specleNoiseTex.Sample( samp, input.texCoord );
		float4 specleWoundClr = specleNoiseClr * float4( 0.8750f, 0.1992f, 0.2226f, 1.0f );

		float4 finalColor = float4( (specleWoundClr.rgb * specleNoiseClr.r) + (lowerWoundClr.rgb * (1.0f - specleNoiseClr.r)), 1.0f );
		return finalColor;
	}

	float4 woundShapeUpper = woundShapeUpperTex.Sample(samp, input.texCoord);
	if ( woundShapeUpper.a > 0.0001f )
		return upperSkinTex.Sample( samp, input.texCoord ) * float4(0.95f, 0.95f, 0.95f, 1.0f);

	return float4(0.0f, 0.0f, 0.0f, 0.0f);
}