//--------------------------------------------------------------------------------------
// File: Tutorial02.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix View;
	matrix Projection;

}

struct VS_INPUT
{
    float4 Pos : POSITION;
    float3 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 Color : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(  VS_INPUT input )
{
    VS_INPUT output = (VS_INPUT)0;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
	output.Color = input.Color;
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input ) : SV_Target
{
	return input.Color;
}
