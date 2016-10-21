
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};

Texture2D tex : register(t0);
SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	return tex.Sample(samp, input.texCoord);
}