
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


Texture2D texture1 : register(t0);
Texture2D texture2 : register(t1);
SamplerState samp  : register(s0);


cbuffer ConstBuffer : register(b0)
{
	float lerpValue;
};


float4 main( PixelInput input ) : SV_TARGET
{
	return lerp( texture1.Sample(samp, input.texCoord), texture2.Sample(samp, input.texCoord), lerpValue );
}