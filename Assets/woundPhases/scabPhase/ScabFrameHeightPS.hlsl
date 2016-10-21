
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


Texture2D scabInsideHeightTex     : register(t0);
Texture2D scabInsideHeightMaskTex : register(t1);
SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 heightClr = float4(1.0f, 1.0f, 1.0f, 0.0f) - scabInsideHeightTex.Sample( samp, input.texCoord );
	//heightClr

	float4 heightMaskClr = scabInsideHeightMaskTex.Sample( samp, input.texCoord );
	heightClr = float4( heightClr.rgb * heightMaskClr.rgb, heightMaskClr.a );

	return heightMaskClr;
	//return scabInsideHeightMaskTex.Sample( samp, input.texCoord );
}