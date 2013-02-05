#include "Common.hlsl"
Texture2D<float4> texDepth : register( t0 );
Texture2D<float4> texShadowMap : register( t1 );
SamplerState samLinear : register( s0 );
SamplerComparisonState samShadow : register( s1 );

#define SHADOW_EPSYLON 0.000005
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

	float ShadowTexSize = ViewportParams.z;
	float DepthCompareVal = ShadowPos.z - SHADOW_EPSYLON;
	//float ShadowDeviceDepth = texShadowMap.Sample( samLinear, ShadowTex.xy ).x;

	float ShadowVal = 0;

	float x, y;
	for (y = -1.5; y <= 1.5; y += 1.0)
		for (x = -1.5; x <= 1.5; x += 1.0)
			 ShadowVal += texShadowMap.SampleCmpLevelZero( samShadow, ShadowTex.xy + float2(x/ShadowTexSize,y/ShadowTexSize), DepthCompareVal );
	
	//ShadowVal += texShadowMap.SampleCmpLevelZero( samShadow, ShadowTex.xy , DepthCompareVal );

	ShadowVal /= 16;

	return float4(ShadowVal, ShadowVal, ShadowVal, ShadowVal);
}