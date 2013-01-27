#include "Common.hlsl"
Texture2D<float4> texDepth : register( t0 );
Texture2D<float4> texShadowMap : register( t1 );
SamplerState samLinear : register( s0 );
SamplerComparisonState samShadow : register( s1 );

#define SHADOW_EPSYLON 0.001
cbuffer ConstantBuffer : register( b0 )
{
	matrix Projection;
	float4 ProjectionParams;
	float4 ViewportParams;
	matrix ShadowMatrix;
}

struct QuadVS_Input
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD0;
};

struct QuadVS_Output
{
    float4 Pos : SV_POSITION;              
    float2 Tex : TEXCOORD0;
};

QuadVS_Output QuadVS( QuadVS_Input Input )

{
    QuadVS_Output Output;
    Output.Pos = Input.Pos;
    Output.Tex = Input.Tex;
    return Output;
}

float4 PS( QuadVS_Output input ) : SV_Target
{
	float DeviceDepth = texDepth.Sample( samLinear, input.Tex ).x;
	float LinearDepth =  GetLinearDepth(DeviceDepth, ProjectionParams.x, ProjectionParams.y) * ProjectionParams.z;

	float2 ScreenPosition = input.Pos.xy;
	ScreenPosition.x /= ViewportParams.x;
	ScreenPosition.y /= ViewportParams.y;
	ScreenPosition.xy = ScreenPosition.xy * 2 -1;
	ScreenPosition.y = -ScreenPosition.y;

	float4 ViewPosition = float4(GetViewPosition(LinearDepth, ScreenPosition, Projection._11, Projection._22).xyz, 1);
	// transform view position into shadow space
	
	float4 ShadowPos = mul(ViewPosition, ShadowMatrix);

	ShadowPos.xyz /= ShadowPos.w;


	ShadowPos.xy = ShadowPos.xy *0.5 + 0.5;
	float2 ShadowTex = ShadowPos.xy; //*0.5 + 0.5;
	ShadowTex.y = 1- ShadowTex.y;

	float ShadowMapSize = ViewportParams.z;

	float2 texelpos = ShadowMapSize * ShadowTex;
        
    // Determine the lerp amounts           
    float2 lerps = frac( texelpos );

	float DepthCompareVal = ShadowPos.z - SHADOW_EPSYLON;
	//float ShadowDeviceDepth = texShadowMap.Sample( samLinear, ShadowTex.xy ).x;

	float sourcevals[4];
	sourcevals[0] = texShadowMap.SampleCmpLevelZero(samShadow, ShadowTex.xy, DepthCompareVal).x;
	sourcevals[1] = texShadowMap.SampleCmpLevelZero(samShadow, ShadowTex.xy + float2(1.0/ShadowMapSize, 0), DepthCompareVal).x;
	sourcevals[2] = texShadowMap.SampleCmpLevelZero(samShadow, ShadowTex.xy + float2(0, 1.0/ShadowMapSize), DepthCompareVal).x;
	sourcevals[3] = texShadowMap.SampleCmpLevelZero(samShadow, ShadowTex.xy + float2(1.0/ShadowMapSize, 1.0/ShadowMapSize), DepthCompareVal).x;

	float ShadowVal = lerp( lerp( sourcevals[0], sourcevals[1], lerps.x ),
                                  lerp( sourcevals[2], sourcevals[3], lerps.x ),
                                  lerps.y );

	return 1-float4(ShadowVal, ShadowVal, ShadowVal, ShadowVal);
}