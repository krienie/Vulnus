
cbuffer ShaderParams : register(b0)
{
	float bias;
	float gain;
	float totalAmplitude;
};

RWStructuredBuffer<float> perlinBuff : register(u0);
RWTexture2D<float4> outputTex        : register(u1);


float getBias( float val, float bias);
float getGain( float val, float gain );


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex )
{
	uint imgWidth;
	uint imgHeight;
	outputTex.GetDimensions(imgWidth, imgHeight);

	//float noiseVal     = perlinBuff[DTid.x + (DTid.y * imgWidth * imgWidth)]; // / totalAmplitude;
	float noiseVal     = perlinBuff[DTid.x + DTid.y * imgWidth] / totalAmplitude;
	noiseVal = getGain( getBias(noiseVal, bias), gain );
	outputTex[DTid.xy] = float4(noiseVal, noiseVal, noiseVal, 1.0f);
}

float getBias( float val, float bias )
{
	return (val / (( ((1.0f / bias) - 2.0f) * (1.0f - val) ) + 1.0f));
}

float getGain( float val, float gain )
{
	if( val < 0.5f )
		return getBias(val * 2.0f, gain) / 2.0f;
	else return getBias(val * 2.0f - 1.0f, 1.0f - gain) / 2.0f + 0.5f;
}
