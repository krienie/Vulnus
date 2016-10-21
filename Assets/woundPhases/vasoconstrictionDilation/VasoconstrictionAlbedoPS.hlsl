
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


Texture2D woundShapeUpperTex : register(t0);
Texture2D lowerSkinTex       : register(t1);

SamplerState samp : register(s0)
{
   Filter = MIN_MAG_MIP_LINEAR;
   AddressU = Wrap;
   AddressV = Wrap;
};


float4 main( PixelInput input ) : SV_TARGET
{
	float4 woundShapeUpper = woundShapeUpperTex.Sample(samp, input.texCoord);
	if ( woundShapeUpper.a > 0.0001f )
		return lowerSkinTex.Sample( samp, input.texCoord );// * float4(0.95f, 0.95f, 0.95f, 1.0f);

	return float4(0.0f, 0.0f, 0.0f, 0.0f);
}
