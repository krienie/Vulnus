
cbuffer MarbleConstants
{
	uint imgWidth;
};


Texture2D<float4> inputTex    : register(t0);
RWTexture2D<float4> outputTex : register(u0);


float getBias( float val, float bias);
float4 getBias4( float4 val, float bias);
float getGain( float val, float gain );
float4 getGain4( float4 val, float gain );


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float4 curPixClr = inputTex.Load( uint3(DTid.x, DTid.y, 0) );
	curPixClr = getGain4( curPixClr, 0.35f );
	outputTex[DTid.xy] = getBias4( curPixClr, 0.9f );

	/*float g = curPixClr.x * 20.0f;
	float grain = g - int(g);
	outputTex[DTid.xy] = float4( grain, grain, grain, 1.0f ) * float4( 0.4882f, 0.3046f, 0.1757f, 1.0f );*/

	/*float phase = float(imgWidth) / 4.0f;
	const float PI = 3.14159265f;

	float4 marbleVal = abs(cos( ((float(DTid.x) / phase) + curPixClr) * PI ));

	outputTex[DTid.xy] = marbleVal;*/
}

float getBias( float val, float bias )
{
	return (val / (( ((1.0f / bias) - 2.0f) * (1.0f - val) ) + 1.0f));
}

float4 getBias4( float4 val, float bias )
{
	return float4( getBias(val.x, bias), getBias(val.y, bias), getBias(val.z, bias), getBias(val.w, bias) );
}

float getGain( float val, float gain )
{
	if( val < 0.5f )
		return getBias(val * 2.0f, gain) / 2.0f;
	else return getBias(val * 2.0f - 1.0f, 1.0f - gain) / 2.0f + 0.5f;
}

float4 getGain4( float4 val, float gain )
{
	return float4( getGain(val.x, gain), getGain(val.y, gain), getGain(val.z, gain), getGain(val.w, gain) );
}
