#include "Common.hlsl"
Texture2D<float4> texDepth : register( t0 );
Texture2D<float4> texShadowMap : register( t1 );
SamplerState samLinear : register( s0 );

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
	


	//if(ShadowPos.z >350)
	//	return float4(0, 1, 1, 1);
	//else
	//	return float4(1, 1, 0, 1);
	

	//return float4(ShadowPos.zzz, 1);

	ShadowPos.xyz /= ShadowPos.w;


	ShadowPos.xy = ShadowPos.xy *0.5 + 0.5;
	float2 ShadowTex = ShadowPos.xy; //*0.5 + 0.5;
	ShadowTex.y = 1- ShadowTex.y;
	float ShadowDeviceDepth = texShadowMap.Sample( samLinear, ShadowTex.xy ).x;
	//float ShadowLinearDepth = GetLinearDepth(ShadowDeviceDepth, ShadowProjectionParams.x, ShadowProjectionParams.y);// * ShadowProjectionParams.z;

	//float LZ = LinearDepth/ProjectionParams.z;
	//return float4(LZ, LZ,LZ, LZ);

	//float SZ =ShadowDeviceDepth;
	//return float4(SZ, SZ,SZ, SZ);
	

	if(ShadowPos.z-0.001 > ShadowDeviceDepth)
		return float4(1, 1, 1, 1);
	else
		return float4(0, 0, 0, 0);
}