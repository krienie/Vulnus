
static float FLOAT_MAX = 3.402823466e+38F;

StructuredBuffer<uint> randomNums;

Texture2D<float4> inputTex;
RWTexture2D<float4> outputTex;

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float4 outputColor = float4( FLOAT_MAX, FLOAT_MAX, FLOAT_MAX, 1.0f );

	uint imgWidth;
	uint imgHeight;
	inputTex.GetDimensions( imgWidth, imgHeight );
	
	uint kernWidth = randomNums[DTid.x + (DTid.y * imgWidth)];
	kernWidth += (kernWidth % 2) - 1;		// make sure the kernel width is an odd number
	int edgeX = kernWidth / 2;

    for ( int kernX = -int(edgeX); kernX <= int(edgeX); ++kernX )
    {
		if ( (DTid.x + kernX) > imgWidth - 1 || (DTid.x + kernX) < 0 )
            continue;

		float4 curPixClr = inputTex.Load( uint3(DTid.x + kernX, DTid.y, 0) );
		outputColor = min( curPixClr, outputColor );
    }

	if ( outputColor.r + outputColor.g + outputColor.b <= 0.0001f )
		outputTex[DTid.xy] = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	else outputTex[DTid.xy] = float4( outputColor.rgb, 1.0f );
}
