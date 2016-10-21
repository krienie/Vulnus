
StructuredBuffer<float2> hullBuff : register(t0);
Texture2D<float4> inputTex        : register(t1);
Texture2D<float4> woundShapeTex   : register(t2);
RWTexture2D<float4> outputTex     : register(u0);


cbuffer ShaderBuff : register(b0)
{
	uint hullBuffSize  : packoffset(c0);
	uint renderSide    : packoffset(c0.y);
	uint thickness     : packoffset(c0.z);
	bool fadeToTexture : packoffset(c0.w);
	float4 colorFrom   : packoffset(c1);
	float4 colorTo     : packoffset(c2);
}


float distFromLineSegment( float2 l1, float2 l2, float2 p );


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float4 woundShapeClr = woundShapeTex.Load( uint3(DTid.x, DTid.y, 0) );

	if ( (renderSide == 0 && woundShapeClr.r + woundShapeClr.g + woundShapeClr.b == 0.0f)			// render outer edge
			|| (renderSide == 1 && woundShapeClr.r + woundShapeClr.g + woundShapeClr.b != 0.0f) )	// render inner edge
	{
		outputTex[DTid.xy] = float4(0.0f, 0.0f, 0.0f, 0.0f);
		return;
	}


	uint imgWidth;
	uint imgHeight;
	outputTex.GetDimensions( imgWidth, imgHeight );

	//get UV coordinate of current pixel
	float2 uvPixelPos = float2( DTid.x / float(imgWidth), DTid.y / float(imgHeight) );

	float minDist = distFromLineSegment( hullBuff[0], hullBuff[1], uvPixelPos );
	
	for ( uint i = 1U; i < hullBuffSize - 1; ++i )
		minDist = min( minDist, distFromLineSegment(hullBuff[i], hullBuff[i + 1], uvPixelPos) );
	minDist = min( minDist, distFromLineSegment(hullBuff[hullBuffSize - 1], hullBuff[0], uvPixelPos) );

	//TODO: define thickness
	float lineThick = thickness * ( 1.0f / float(imgWidth) );
	if ( minDist <= lineThick )
	{
		float4 lerpTo = colorTo;
		if ( fadeToTexture )
			lerpTo = inputTex.Load( uint3(DTid.x, DTid.y, 0) );

		float4 newColor = lerp( colorFrom, lerpTo, minDist / lineThick );
		outputTex[DTid.xy] = newColor;
	} else
	{
		float4 lerpTo = colorTo;
		if ( fadeToTexture )
			lerpTo = inputTex.Load( uint3(DTid.x, DTid.y, 0) );
		outputTex[DTid.xy] = lerpTo;
	}
}


float distFromLineSegment( float2 l1, float2 l2, float2 p )
{
	float d = dot(p - l2, l2 - l1);
	if ( d > 0 )
	{
		float2 l2MinP = l2 - p;
		return sqrt( dot(l2MinP, l2MinP) );
	}

	d = dot(p - l1, l1 - l2); 
	if ( d > 0 )
	{
		float2 l1MinP = l1 - p;
		return sqrt( dot(l1MinP, l1MinP) );
	}

	float2 lineSeg = l2 - l1;
	lineSeg = float2( lineSeg.y, -lineSeg.x );
	lineSeg = normalize(lineSeg);
	float2 pToLine = l1 - p;
	return abs( dot(lineSeg, pToLine) );
}
