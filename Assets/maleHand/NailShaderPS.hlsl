
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
	float3 lightDir : LIGHTDIR;
	float3 camDir   : CAMDIR;
};


cbuffer tracerParams : register(b0)
{
	float4 aLightClr;
	float dLightIntensity;
};


Texture2D texColor : register(t0);
Texture2D texNorm  : register(t1);
SamplerState samp : register(s0);


float4 main( PixelInput input ) : SV_TARGET
{
	// unpack normal from texture
	float3 normal = texNorm.Sample( samp, input.texCoord ).rgb;
	normal = normalize( (normal * 2.0f) - 1.0f );

	// calculate lighting
	float3 nDotl = saturate( dot(normal, input.lightDir) );

	// calculate reflectance
	float3 h = normalize(input.lightDir + input.camDir);
	float specLighting = pow( saturate(dot(h, normal)), 6 );

	float4 diffuseClr = texColor.Sample(samp, input.texCoord);
	float4 finalColor = aLightClr + (diffuseClr * float4(nDotl, 1.0f) * dLightIntensity) + (specLighting * 0.75f);
	return saturate( finalColor );
}
