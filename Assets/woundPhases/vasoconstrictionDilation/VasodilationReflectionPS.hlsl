
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


Texture2D blendMaskTex     : register(t0);
Texture2D noiseMapTex      : register(t1);
Texture2D noiseMapMaskTex  : register(t2);
SamplerState samp          : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	float4 blendMask = blendMaskTex.Sample( samp, input.texCoord );
	if ( blendMask.r < 0.001f )
		return float4( 0.0f, 0.0f, 0.0f, 0.0f );
	else return noiseMapTex.Sample( samp, input.texCoord ) * noiseMapMaskTex.Sample( samp, input.texCoord );
}