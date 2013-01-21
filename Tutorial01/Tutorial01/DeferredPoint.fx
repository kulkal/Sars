#include "Common.hlsl"
Texture2D<float4> texWorldNormal : register( t0 );
Texture2D<float4> texDepth : register( t1 );
SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
	float4 vLightPos;
	float4 vLightColor;
	matrix View;
	matrix Projection;
	float4 ProjectionParams;
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
	float3 ViewNormal = texWorldNormal.Sample( samLinear, input.Tex ).xyz;
	
	float DeviceDepth = texDepth.Sample( samLinear, input.Tex ).x;
	float LinearDepth =  GetLinearDepth(DeviceDepth, ProjectionParams.x, ProjectionParams.y) * ProjectionParams.z;

	float2 ScreenPosition = input.Tex.xy * 2 -1;
	ScreenPosition.y = -ScreenPosition.y;
	float3 ViewPosition = GetViewPosition(LinearDepth, ScreenPosition, Projection._11, Projection._22);

	float3 LightDir = vLightPos.xyz - ViewPosition;
	float LightDist = length(LightDir);
	LightDir = normalize(LightDir);
	float LightRange = vLightColor.w;
	float Attenuation = saturate(1-LightDist/LightRange);
	Attenuation*=Attenuation;

	float NdotL = dot(LightDir,ViewNormal);
	return float4(NdotL *vLightColor.xyz *Attenuation , 1);

	if(LightDist < 100)
		return float4(1, 0, 0, 1);
	else
		return float4(0, 0, 1, 1);
		
	/*
	if(ViewPosition.x < 150 ) 
		return float4(1, 0, 0, 1);
	else
		return float4(0, 0, 1, 1);
		*/
	//return  float4(saturate(  * vLightColor.xyz), 1.f) * Attenuation;
}