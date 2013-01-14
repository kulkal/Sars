//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "GpuSkinning.hlsl"
#include "VSPSInput.hlsl"

Texture2D txDiffuse ;
SamplerState samLinear : register( s0 );
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
}

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(  VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
#if GPUSKINNING
	float4x4 BoneMat = CalcBoneMatrix(input.Bones, input.Weights);
	output.Pos = float4(input.Pos, 1.f);
	output.Pos = mul( output.Pos, World );
	output.Pos = mul(output.Pos, BoneMat);
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection);

	output.Norm = mul( input.Norm, World );
	output.Norm = mul(output.Norm, BoneMat);
#else
	output.Pos = float4(input.Pos, 1.f);
    output.Pos = mul( output.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	output.Norm = mul( input.Norm, World );
#endif
#if TEXCOORD
	output.Tex = input.Tex;
#endif

    return output;
}

struct PS_OUTPUT
{
    float4 color : COLOR0;    
    float4 normal : COLOR1;    
};
 
//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
PS_OUTPUT PS( PS_INPUT input ) : SV_Target
{
	 float4 finalColor = 0;
    
    //do NdotL lighting for 2 lights
    //for(int i=0; i<2; i++)
    //{
    //    finalColor += saturate( dot( (float3)vLightDir[i],input.Norm) * vLightColor[i] );
    //}
    finalColor.a = 1;

	PS_OUTPUT output;
	output.normal = float4(input.Norm, 1.f);
#if TEXCOORD
	output.color = float4(1.f, 0.f, 0.f, 1.f);
#else
	output.color = float4(0.f, 0.f, 1.f, 1.f);

#endif
	return output;
}

