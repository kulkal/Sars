#include "Common.hlsl"

Texture2D<float4> texWorldNormal : register( t0 );
Texture2D<float4> texDepth : register( t1 );
Texture2D<float4> texShadowMap : register( t2 );
SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
	float4 vLightDir;
	float4 vLightColor;
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
	float3 LightDir = - vLightDir.xyz;
	float3 ViewNormal = normalize(texWorldNormal.Sample( samLinear, input.Tex ).xyz);
	float4 Shadow = texShadowMap.Sample(samLinear, input.Tex);

	float NdotL = dot(LightDir, ViewNormal);
	

	float3 Specular = CalcBlinPhong(LightDir, ViewNormal, 100);
	float4 Light =  saturate( NdotL * vLightColor ) + float4(Specular.xyz * vLightColor.xyz, 1);

	return Light * Shadow;
}