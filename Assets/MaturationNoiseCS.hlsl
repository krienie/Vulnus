
Texture2D<float4> inputTex    : register(t0);
RWTexture2D<float4> outputTex : register(u0);


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint imgWidth;
	uint imgHeight;
	inputTex.GetDimensions(imgWidth, imgHeight);


	float kernel1[9] = { 1.0f, 2.0f, 1.0f,
						 0.0f, 0.0f, 0.0f,
						-1.0f, -2.0f, -1.0f };

	float kernel2[9] = { 2.0f, 1.0f, 0.0f,
						 1.0f, 0.0f, -1.0f,
						 0.0f, -1.0f, -2.0f };

	float kernel3[9] = { 1.0f, 0.0f, -1.0f,
						 2.0f, 0.0f, -2.0f,
						 1.0f, 0.0f, -1.0f };

	float kernel4[9] = { 0.0f, -1.0f, -2.0f,
						 1.0f, 0.0f, -1.0f,
						 2.0f, 1.0f, 0.0f };

	int edgeSize   = 1;
	int kernWidth  = 3;
	/* loop through kernel elements */
	float4 horVal1  = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 horVal2  = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 vertVal1 = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 vertVal2 = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 diagVal1 = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 diagVal2 = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 diagVal3 = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 diagVal4 = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	for ( int kernY = -edgeSize; kernY <= edgeSize; ++kernY )
    {
        for ( int kernX = -edgeSize; kernX <= edgeSize; ++kernX )
        {
			uint kernIdx      = (kernY + edgeSize) * kernWidth + (kernX + edgeSize);
			float4 curPixClr = inputTex.Load( uint3(DTid.x + kernX, DTid.y + kernY, 0) );		
			
			horVal1  += curPixClr * kernel1[kernIdx];
			horVal2  += curPixClr * kernel1[24 - kernIdx];
			vertVal1 += curPixClr * kernel2[kernIdx];
			vertVal2 += curPixClr * kernel2[24 - kernIdx];
			diagVal1 += curPixClr * kernel3[kernIdx];
			diagVal2 += curPixClr * kernel3[8 - kernIdx];
			diagVal3 += curPixClr * kernel4[kernIdx];
			diagVal4 += curPixClr * kernel4[8 - kernIdx];
        }
    }

	float4 horEdge  = max(horVal1, horVal2);
	float4 vertEdge = max(vertVal1, vertVal2);
	float4 diagEdge = max( max(diagVal1, diagVal2), max(diagVal3, diagVal4) );
	float4 edgeVal = max( horEdge, max(vertEdge, diagEdge) );

	outputTex[DTid.xy] = float4( edgeVal.rgb, 1.0f );
}
