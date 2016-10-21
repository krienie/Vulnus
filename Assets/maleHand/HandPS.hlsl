
struct PixelInput
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXTURE;
	float3 lightDir : LIGHTDIR;
	float3 camDir   : CAMDIR;
};


cbuffer pixelParams : register(b0)
{
	float4 aLightClr;
	float dLightIntensity;
};


Texture2D texHandClr     : register(t0);
Texture2D texHandNorm    : register(t1);
Texture2D texHandSpec    : register(t2);
Texture2D texPatchClr    : register(t4);
Texture2D texPatchNorm   : register(t5);
Texture2D texPatchSpec   : register(t6);
SamplerState samp : register(s0);

//Texture2D tempTex : register(t7);

float4 main( PixelInput input ) : SV_TARGET
{
	//return tempTex.Sample( samp, input.texCoord );
	//return texPatchSpec.Sample( samp, input.texCoord );


	// sample normal value
	float4 normal4 = texPatchNorm.Sample(samp, input.texCoord);
	if ( normal4.a < 0.9f )
		normal4 = texHandNorm.Sample(samp, input.texCoord);

	// unpack normal from texture
	float3 normal = normal4.rgb;
	normal = normalize( (normal * 2.0f) - 1.0f );

	// decrease normal detail
	float factor = 0.75f;
	normal = normalize( (normal * factor) + (float3(0.0f, 0.0f, 1.0f) * (1.0f - factor)) );

	// calculate lighting
	float nDotl = saturate( dot(normal, input.lightDir) );

	// calculate reflectance
	float3 h = normalize(input.lightDir + input.camDir);
	float specLighting = pow( saturate(dot(h, normal)), 6 );

	// sample specular map
	float4 specMapClr = texPatchSpec.Sample( samp, input.texCoord );
	if ( specMapClr.a < 0.9f )
		specMapClr = texHandSpec.Sample(samp, input.texCoord);

	// get hand/patch color
	float4 woundPatchClr = texPatchClr.Sample( samp, input.texCoord );
	float4 handSkinClr   = texHandClr.Sample( samp, input.texCoord );
	float4 woundClr = float4( (woundPatchClr.rgb * woundPatchClr.a) + (handSkinClr.rgb * (1.0f - woundPatchClr.a)), 1.0f );
	//float4 woundClr = woundPatchClr;

	float4 finalColor = aLightClr + (woundClr * nDotl * dLightIntensity) + (specLighting * specMapClr * 0.25f);
	return saturate( finalColor );
}
