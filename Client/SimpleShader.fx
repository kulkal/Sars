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
	float4 vLightDir[2];
	float4 vLightColor[2];

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
	output.Norm = normalize(mul(output.Norm, BoneMat));
#else
	output.Pos = float4(input.Pos, 1.f);
    output.Pos = mul( output.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	output.Norm = normalize(mul( input.Norm, World ));
#endif
#if TEXCOORD
	output.Tex = input.Tex;
#endif

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input ) : SV_Target
{
	 float4 finalColor = 0;
    
    //do NdotL lighting for 2 lights
    for(int i=0; i<2; i++)
    {
        finalColor += saturate( dot( (float3)vLightDir[i],input.Norm) * vLightColor[i] );
    }
    finalColor.a = 1;
#if TEXCOORD
    return  float4(0.1f, 0.1f, 0.1f, 1.f) + finalColor*txDiffuse.Sample( samLinear, input.Tex );
#else
    return  finalColor;
#endif

}

