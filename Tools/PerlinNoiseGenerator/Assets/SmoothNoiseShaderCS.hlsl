
cbuffer NoiseParams : register(b0)
{
	float frequency;
	float amplitude;
	float2 padding;
	int period;
	uint imgWidth;
	uint imgHeight;
}

StructuredBuffer<float> whiteNoise   : register(t0);
RWStructuredBuffer<float> perlinBuff : register(u0);


float cubicInterp( float v0, float v1, float v2, float v3, float x );


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex )
{
	//calculate the vertical sampling indices
	int y1 = (DTid.y / period) * period;
	int y0 = y1 < period ? imgHeight - period + y1 : y1 - period;		// wrap around
	int y2 = (y1 + period) % imgHeight;
	int y3 = (y2 + period) % imgHeight;
	float vertBlend = float(DTid.y - y1) * frequency;


	//calculate the horizontal sampling indices
	int x1 = (DTid.x / period) * period;
	int x0 = x1 < period ? imgWidth - period + x1 : x1 - period;		// wrap around
	int x2 = (x1 + period) % imgWidth;
	int x3 = (x2 + period) % imgWidth;
	float horBlend = float(DTid.x - x1) * frequency;

	// interpolate rows
	float row1 = cubicInterp( whiteNoise[x0 + (y0 * imgWidth)], whiteNoise[x1 + (y0 * imgWidth)],
								whiteNoise[x2 + (y0 * imgWidth)], whiteNoise[x3 + (y0 * imgWidth)], horBlend );
	float row2 = cubicInterp( whiteNoise[x0 + (y1 * imgWidth)], whiteNoise[x1 + (y1 * imgWidth)],
								whiteNoise[x2 + (y1 * imgWidth)], whiteNoise[x3 + (y1 * imgWidth)], horBlend );
	float row3 = cubicInterp( whiteNoise[x0 + (y2 * imgWidth)], whiteNoise[x1 + (y2 * imgWidth)],
								whiteNoise[x2 + (y2 * imgWidth)], whiteNoise[x3 + (y2 * imgWidth)], horBlend );
	float row4 = cubicInterp( whiteNoise[x0 + (y3 * imgWidth)], whiteNoise[x1 + (y3 * imgWidth)],
								whiteNoise[x2 + (y3 * imgWidth)], whiteNoise[x3 + (y3 * imgWidth)], horBlend );

	perlinBuff[DTid.x + DTid.y * imgWidth] += cubicInterp( row1, row2, row3, row4, vertBlend ) * amplitude;
}


float cubicInterp( float v0, float v1, float v2, float v3, float x )
{
	float p = (v3 - v2) - (v0 - v1);
	float q = (v0 - v1) - p;
	float r = v2 - v0;
	float s = v1;

	return saturate( p * pow(x, 3) + q * pow(x, 2) + r * x + s );
}



//calculate the vertical sampling indices
	/*int y1 = (DTid.y / period) * period;
	int y0 = y1 < period ? imgHeight - period + y1 : y1 - period;		// wrap around
	int y2 = (y1 + period) % imgHeight;
	int y3 = (y2 + period) % imgHeight;
	float vertBlend = float(DTid.y - y1) * frequency;

	perlinBuff[DTid.x + DTid.y * 512] = whiteNoise[DTid.x + DTid.y * 512];

	//calculate the horizontal sampling indices
	int x1 = (DTid.x / period) * period;
	int x0 = x1 < period ? imgWidth - period + x1 : x1 - period;		// wrap around
	int x2 = (x1 + period) % imgWidth;
	int x3 = (x2 + period) % imgWidth;
	float horBlend = float(DTid.x - x1) * frequency;

	// interpolate rows
	float row1 = cubicInterp( whiteNoise[x0 + (y0 * imgWidth)], whiteNoise[x1 + (y0 * imgWidth)],
								whiteNoise[x2 + (y0 * imgWidth)], whiteNoise[x3 + (y0 * imgWidth)], horBlend );
	float row2 = cubicInterp( whiteNoise[x0 + (y1 * imgWidth)], whiteNoise[x1 + (y1 * imgWidth)],
								whiteNoise[x2 + (y1 * imgWidth)], whiteNoise[x3 + (y1 * imgWidth)], horBlend );
	float row3 = cubicInterp( whiteNoise[x0 + (y2 * imgWidth)], whiteNoise[x1 + (y2 * imgWidth)],
								whiteNoise[x2 + (y2 * imgWidth)], whiteNoise[x3 + (y2 * imgWidth)], horBlend );
	float row4 = cubicInterp( whiteNoise[x0 + (y3 * imgWidth)], whiteNoise[x1 + (y3 * imgWidth)],
								whiteNoise[x2 + (y3 * imgWidth)], whiteNoise[x3 + (y3 * imgWidth)], horBlend );

	perlinBuff[DTid.x * DTid.y] += cubicInterp( row1, row2, row3, row4, vertBlend ) * amplitude;*/
