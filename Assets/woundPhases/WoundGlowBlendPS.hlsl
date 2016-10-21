
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


Texture2D keyframeTex    : register(t0);
Texture2D preRenderTex   : register(t1);
Texture2D woundShapeTex  : register(t2);
SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 woundShapeClr = woundShapeTex.Sample( samp, input.texCoord );
	if ( woundShapeClr.r < 0.001f )								// outside wound
		return preRenderTex.Sample( samp, input.texCoord );
	else return keyframeTex.Sample( samp, input.texCoord );		// inside wound
}
