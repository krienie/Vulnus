
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


Texture2D blendMaskTex  : register(t0);
Texture2D heightMaskTex : register(t1);
SamplerState samp       : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 blendMask = blendMaskTex.Sample( samp, input.texCoord );
	if ( blendMask.r < 0.001f )
		return float4( 0.0f, 0.0f, 0.0f, 0.0f );
	else return heightMaskTex.Sample( samp, input.texCoord );
}