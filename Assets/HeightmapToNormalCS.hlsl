
Texture2D<float4> inputTex;
RWTexture2D<float4> outputTex;

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint imgWidth;
	uint imgHeight;
	inputTex.GetDimensions( imgWidth, imgHeight );

	int y1 = int(DTid.y);
	int y0 = y1 - 1 < 0 ? int(imgHeight - 1) : y1 - 1;
	int y2 = uint(y1 + 1) % imgHeight < uint(y1) ? 0 : y1 + 1;

	int x1 = int(DTid.x);
	int x0 = x1 - 1 < 0 ? int(imgWidth - 1) : x1 - 1;
	int x2 = uint(x1 + 1) % imgWidth < uint(x1) ? 0 : x1 + 1;

	// get height values
	float alpha = 1.0f;
	//TODO: gebruik SampleLevel hiervoor
	float4 h1 = inputTex.Load( uint3(x2, y1, 0) );
	float4 h2 = inputTex.Load( uint3(x0, y1, 0) );
	float z1 = alpha * (h1.z - h2.z);

	h1 = inputTex.Load( uint3(x1, y2, 0) );
	h2 = inputTex.Load( uint3(x1, y0, 0) );
	float z2 = alpha * (h1.z - h2.z);

	// compute normal for current pixel
	float3 a = float3( 1.0f, 0.0f, z1 );
	float3 b = float3( 0.0f, 1.0f, z2 );
	float3 normal = cross(a, b);

	// pack calculated normal in rgb values
	normal = (normal + 1.0f) / 2.0f;

	outputTex[DTid.xy] = float4(normal, 1.0f);
}
