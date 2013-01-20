#include "Common.hlsl"
Texture2D<float4> texSceneColor : register( t0 );
Texture2D<float4> texLit : register( t1 );
SamplerState samLinear : register( s0 );

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
    float4 SceneColor = texSceneColor.Sample( samLinear, input.Tex );
	float4 Lit = texLit.Sample( samLinear, input.Tex );
	
	return SceneColor * Lit;
}