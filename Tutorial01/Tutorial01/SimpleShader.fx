//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
Buffer<float4> BoneMatrices;
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


struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Norm : NORMAL;
#if TEXCOORD
	float2 Tex : TEXCOORD0;
#endif
#if GPUSKINNING
	float4 Weights: WEIGHTS;
	uint4 Bones : BONES;
#endif
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Norm : NORMAL;
#if TEXCOORD
	float2 Tex : TEXCOORD0;
#endif

};

#if GPUSKINNING
#define MAX_BONELINK 4
float4x4 CalcBoneMatrix(VS_INPUT input)
{
	float4x4 TotalMat = (float4x4)0;

	for(int i=0;i<MAX_BONELINK;i++)
	{
		uint iBone = input.Bones[i] * 4;
		float4 row1 = BoneMatrices.Load( iBone );
		float4 row2 = BoneMatrices.Load( iBone + 1 );
		float4 row3 = BoneMatrices.Load( iBone + 2 );
		float4 row4 = BoneMatrices.Load( iBone + 3 );
		float4x4 Mat = float4x4( row1, row2, row3, row4 );
		
		TotalMat += Mat* input.Weights[i];
	}

	return TotalMat;
}
#endif

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(  VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
#if GPUSKINNING
	float4x4 BoneMat = CalcBoneMatrix(input);
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

