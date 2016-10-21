
// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float4 position : WORLDPOS;
	float2 texCoord : TEXTURE;
	float3 normal   : NORMAL;
	float3 lightDir : LIGHTDIR;
	float3 camDir   : CAMDIR;
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


cbuffer hullParams : register(b0)
{
	float tessellationFactor;
};


#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONSTANT_DATA_OUTPUT Output;

	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = tessellationFactor;

	[unroll]
	for ( uint i = 0; i < 3; ++i )
	{
		Output.texCoord[i] = ip[i].texCoord;
		Output.normal[i]   = ip[i].normal;
	}

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT output;

	output.vPosition = ip[i].position.xyz;
	output.lightDir  = ip[i].lightDir;
	output.camDir    = ip[i].camDir;

	return output;
}
