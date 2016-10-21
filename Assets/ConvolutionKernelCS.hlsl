
cbuffer KernelBuffer : register(b0)
{
	float4 kernel[7];
	float4 padding;
	uint width;
	uint height;
	uint edgeX;
	uint edgeY;
	uint imgWidth;
	uint imgHeight;
};


Texture2D<float4> inputTex;
RWTexture2D<float4> outputTex;

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float4 totalPixVal = float4( 0.0f, 0.0f, 0.0f, 0.0f );
	for ( int kernY = -int(edgeY); kernY <= int(edgeY); ++kernY )
    {
        for ( int kernX = -int(edgeX); kernX <= int(edgeX); ++kernX )
        {
			uint kernIdx      = (kernY + edgeY) * width + (kernX + edgeX);
			float kernelValue = ((float[4])(kernel[kernIdx / 4]))[kernIdx % 4];

			float4 curPixClr = inputTex.Load( uint3(DTid.x + kernX, DTid.y + kernY, 0) );		
			totalPixVal += curPixClr * kernelValue;
        }
    }

	outputTex[DTid.xy] = totalPixVal;
}
