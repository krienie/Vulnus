
struct VS_OUTPUT
{
	float4 position : WORLDPOS;
	float2 texCoord : TEXTURE;
	float3 normal   : NORMAL;
	float3 lightDir : LIGHTDIR;
	float3 camDir   : CAMDIR;
};

cbuffer vertParams : register(b0)
{
	float4x4 worldMat;
	float4x4 worldMatInvTrans;
	float3 camPosition;
	float padding;
	float3 dLightDir;
}


VS_OUTPUT main( float4 Pos : POSITION, float3 normal : NORMAL, float2 texCoord0 : TEXCOORD0, float4 tangent : TANGENT )
{
	VS_OUTPUT output;
	
	output.position = mul( worldMat, Pos );

	//TODO: move normal code to the domain shader?
	float4 modelNormal  = mul( worldMatInvTrans, float4(normal, 0.0f) );
	float4 modelTangent = mul( worldMatInvTrans, float4(tangent.xyz, 0.0f) );
	float3 bitangent    = tangent.w * cross(modelNormal.xyz, modelTangent.xyz);

	output.texCoord = texCoord0;
	output.lightDir = float3( dot(modelTangent.xyz, -dLightDir),
								dot(bitangent, -dLightDir),
								dot(modelNormal.xyz, -dLightDir) );

	float4 worldPos   = mul( worldMat, Pos );
	float3 tempCamDir = normalize(camPosition - worldPos.xyz);
	output.camDir     = float3( dot(modelTangent.xyz, tempCamDir),
								dot(bitangent, tempCamDir),
								dot(modelNormal.xyz, tempCamDir) );

	float4 normal4 = mul( worldMatInvTrans, float4(normal, 0.0f) );
	output.normal = normal4.xyz;

	return output;
}
