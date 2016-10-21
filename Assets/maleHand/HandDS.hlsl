
struct DS_OUTPUT
{
	float4 vPosition : SV_POSITION;
	float2 texCoord  : TEXTURE;
	float3 lightDir  : LIGHTDIR;
	float3 camDir    : CAMDIR;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float3 vPosition : WORLDPOS;
	float3 lightDir  : LIGHTDIR;
	float3 camDir    : CAMDIR;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]	: SV_TessFactor;
	float InsideTessFactor	: SV_InsideTessFactor;

	float2 texCoord[3] : TEXTURE;
	float3 normal[3]   : NORMAL;
};


cbuffer domainParams : register(b0)
{
	float4x4 viewProjMat;
	float displacementScale;
};

Texture2D texHandHeight  : register(t3);
Texture2D texPatchHeight : register(t7);
SamplerState samp : register(s0);


float2 baryInterp( float2 v0, float2 v1, float2 v2, float3 barycentric );
float2 baryInterp( float2 v[3], float3 barycentric );
float3 baryInterp( float3 v0, float3 v1, float3 v2, float3 barycentric );
float3 baryInterp( float3 v[3], float3 barycentric );
float3 getDisplacement( float2 texCoord, float3 worldNormal );


#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch )
{
	float3 pos = baryInterp( patch[0].vPosition, patch[1].vPosition, patch[2].vPosition, domain );

	float2 texCoord  = baryInterp( input.texCoord, domain );
	float3 worldNorm = baryInterp( input.normal, domain );
	pos += getDisplacement( texCoord, worldNorm );

	DS_OUTPUT output;
	output.vPosition = mul( viewProjMat, float4(pos, 1.0f) );
	output.texCoord  = texCoord;
	output.lightDir  = baryInterp( patch[0].lightDir, patch[1].lightDir, patch[2].lightDir, domain );
	output.camDir    = baryInterp( patch[0].camDir, patch[1].camDir, patch[2].camDir, domain );

	return output;
}


float2 baryInterp( float2 v0, float2 v1, float2 v2, float3 barycentric )
{
    return barycentric.z * v0 + barycentric.x * v1 + barycentric.y * v2;
}

float2 baryInterp( float2 v[3], float3 barycentric )
{
    return baryInterp(v[0], v[1], v[2], barycentric);
}

float3 baryInterp( float3 v0, float3 v1, float3 v2, float3 barycentric )
{
    return barycentric.z * v0 + barycentric.x * v1 + barycentric.y * v2;
}

float3 baryInterp( float3 v[3], float3 barycentric )
{
    return baryInterp(v[0], v[1], v[2], barycentric);
}


float3 getDisplacement( float2 texCoord, float3 worldNormal )
{
	// Skip displacement sampling if 0 multiplier
	if ( displacementScale == 0.0f )
		return float3(0.0f, 0.0f, 0.0f);

	// Sample height map
	float4 heightVal = texPatchHeight.SampleLevel(samp, texCoord, -1.0f);
	if ( heightVal.a < 0.9f )
		heightVal = texHandHeight.SampleLevel(samp, texCoord, -1.0f);

	// Return offset along normal.
	return heightVal.r * displacementScale * worldNormal;
}
