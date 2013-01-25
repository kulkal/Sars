#include "Common.hlsl"
Texture2D<float4> texDepth : register( t0 );
Texture2D<float4> texShadowMap : register( t1 );
SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
	float4x4 ShadowMatrix;
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

	float2 ScreenPosition = input.Tex.xy * 2 -1;
	ScreenPosition.y = -ScreenPosition.y;
	float3 ViewPosition = GetViewPosition(LinearDepth, ScreenPosition, Projection._11, Projection._22);

	// transform view position into shadow space

	return float(1, 0, 0, 0);
}