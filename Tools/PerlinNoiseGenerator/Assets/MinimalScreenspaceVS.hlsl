
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
};


PixelInput main( float4 Pos : POSITION, float3 normal : NORMAL, float2 texCoord0 : TEXCOORD0 )
{
	PixelInput output;
	output.position = Pos;
	output.texCoord = texCoord0;

	return output;
}
