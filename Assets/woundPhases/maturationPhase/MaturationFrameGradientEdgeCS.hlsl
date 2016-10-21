
StructuredBuffer<float2> hullBuff        : register(t0);
Texture2D<float4> skinClrTex             : register(t1);
Texture2D<float4> handReflectTex         : register(t2);
Texture2D<float4> handNormalTex          : register(t3);
Texture2D<float4> woundShapeTex          : register(t4);
Texture2D<float4> reflectionNoiseTex     : register(t5);
Texture2D<float4> reflectionNoiseMaskTex : register(t6);
Texture2D<float4> noiseTex               : register(t7);
RWTexture2D<float4> albedoOutputTex      : register(u0);
RWTexture2D<float4> normalOutputTex      : register(u1);
RWTexture2D<float4> reflectOutputTex     : register(u2);


cbuffer ShaderBuff : register(b0)
{
	float phaseThickness;
	float innerWoundBorder;
	float outerWoundBorder;
	uint hullBuffSize;
}


float distFromLineSegment( float2 l1, float2 l2, float2 p );


[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint imgWidth;
	uint imgHeight;
	albedoOutputTex.GetDimensions( imgWidth, imgHeight );

	//get UV coordinate of current pixel
	float2 uvPixelPos = float2( DTid.x / float(imgWidth), DTid.y / float(imgHeight) );

	float minDist = distFromLineSegment( hullBuff[0], hullBuff[1], uvPixelPos );	
	for ( uint i = 1U; i < hullBuffSize - 1; ++i )
		minDist = min( minDist, distFromLineSegment(hullBuff[i], hullBuff[i + 1], uvPixelPos) );
	minDist = min( minDist, distFromLineSegment(hullBuff[hullBuffSize - 1], hullBuff[0], uvPixelPos) );


	uint handImgWidth;
	uint handImgHeight;
	skinClrTex.GetDimensions( handImgWidth, handImgHeight );

	// calculate pixel position in skin color texture
	uint xPixelPos = uint(float(DTid.x * handImgWidth) / float(imgWidth)) + 1U;
	uint yPixelPos = uint(float(DTid.y * handImgHeight) / float(imgHeight)) + 1U;

	float4 woundShapeClr = woundShapeTex.Load( uint3(DTid.x, DTid.y, 0U) );
	if ( woundShapeClr.r + woundShapeClr.g + woundShapeClr.b == 0.0f )
	{
		//outside the wound shape
		albedoOutputTex[DTid.xy]  = skinClrTex.Load( uint3(xPixelPos, yPixelPos, 0) );
		normalOutputTex[DTid.xy]  = float4( 0.0f, 0.0f, 0.0f, 0.0f );
		reflectOutputTex[DTid.xy] = float4( 0.0f, 0.0f, 0.0f, 0.0f );
		return;
	} 


	if ( minDist <= outerWoundBorder )
	{
		albedoOutputTex[DTid.xy] = skinClrTex.Load( uint3(xPixelPos, yPixelPos, 0) );
		//albedoOutputTex[DTid.xy] = float4(0.0f, 0.0f, 1.0f, 1.0f);
	} else if ( minDist <= innerWoundBorder )
	{
		// calculate color Lerp value
		minDist       = minDist - outerWoundBorder;
		float lerpVal = minDist / (innerWoundBorder - outerWoundBorder);

		// calculate color
		float4 woundBorderClr = float4( 0.7812f, 0.0f, 0.0f, 1.0f );
		float4 skinClr        = skinClrTex.Load( uint3(xPixelPos, yPixelPos, 0) );
		albedoOutputTex[DTid.xy] = lerp( skinClr, woundBorderClr, lerpVal * 0.5f );

		// calculate normal
		lerpVal = 1.0f - (minDist / innerWoundBorder);
		float4 handNormalClr = handNormalTex.Load( uint3(xPixelPos, yPixelPos, 0) );
		float3 newPixClr = lerp( handNormalClr.rgb, float3(0.5f, 0.5f, 1.0f), 1.0f - lerpVal );
		normalOutputTex[DTid.xy] = float4( newPixClr, 1.0f );

		// calculate reflectance
		float4 handReflectClr = handReflectTex.Load( uint3(xPixelPos, yPixelPos, 0) );
		newPixClr = lerp( handReflectClr.rgb, float3(0.0f, 0.0f, 0.0f), 1.0f - lerpVal );
		reflectOutputTex[DTid.xy] = float4( newPixClr, 1.0f );
	} else
	{
		float4 reflectionNoiseClr = reflectionNoiseTex.Load( uint3(DTid.x, DTid.y, 0) );
		float4 reflectionNoiseMaskClr = reflectionNoiseMaskTex.Load( uint3(DTid.x, DTid.y, 0) );

		// inside wound
		/*float3 woundInsideClr = reflectionNoiseClr.rgb * float3(0.25f, 0.25f, 0.25f)
									+ reflectionNoiseMaskClr.rgb * float3( 0.15f, 0.15f, 0.15f )
									+ float3( 0.4765f, 0.2226f, 0.1992f ); //float3( 0.6640f, 0.3750f, 0.3984f );*/
		// calculate color Lerp value
		minDist       = minDist - innerWoundBorder;
		float lerpVal = minDist / (phaseThickness - innerWoundBorder);


		float4 skinClr        = skinClrTex.Load( uint3(xPixelPos, yPixelPos, 0) );
		float3 redSkinClr     = lerp( float3(0.7812f, 0.0f, 0.0f), skinClr.rgb, 0.5f );
		
		float3 whiteSkinClr   = lerp( float3(0.7812f, 0.406f, 0.406f), skinClr.rgb, 0.5f );
		//float4 insideNoiseClr = noiseTex.Load( uint3(DTid.x, DTid.y, 0) );
		//whiteSkinClr = lerp( whiteSkinClr, float3(0.2383f, 0.1055f, 0.1055f), insideNoiseClr.r );

		float3 woundInsideClr = lerp( redSkinClr, whiteSkinClr, lerpVal * 2.0f );

		


		albedoOutputTex[DTid.xy]  = float4( woundInsideClr, 1.0f );
		normalOutputTex[DTid.xy]  = float4( 0.5f, 0.5f, 1.0f, 1.0f );
		reflectOutputTex[DTid.xy] = float4( 0.0f, 0.0f, 0.0f, 1.0f );//reflectionNoiseClr;
	}
}


float distFromLineSegment( float2 l1, float2 l2, float2 p )
{
	float2 lineSeg = l2 - l1;
	lineSeg = float2( lineSeg.y, -lineSeg.x );
	lineSeg = normalize(lineSeg);
	float2 pToLine = l1 - p;

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

	return abs( dot(lineSeg, pToLine) );
}
