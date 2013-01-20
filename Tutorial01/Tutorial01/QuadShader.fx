#include "Common.hlsl"
Texture2D<float4> texWorldNormal : register( t0 );
Texture2D<float4> texDepth : register( t1 );
SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
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
#ifdef VIS_NORMAL
    return texWorldNormal.Sample( samLinear, input.Tex );
#elif VIS_DEPTH
	float DeviceZ = texDepth.Sample( samLinear, input.Tex ).x;
	
	// ProjectionParams.x = zf/(zf - zn)
	// ProjectionParams.y = zn/(zn - zf)
	//float LinearZ = ProjectionParams.y/(DeviceZ - ProjectionParams.x);
	float LinearZ = GetLinearDepth(DeviceZ, ProjectionParams.x, ProjectionParams.y);
	// Projection._34 = zn*zf/(zn-zf)
	// Projection._33 = zf/(zn-zf)
	//float A = Projection._34/f;
	//float B = Projection._33;
	//float A = ProjectionParams.z;
	//float B = ProjectionParams.w;

	//float LInearZ = A/(DeviceZ + B);
	
	
	return  float4(LinearZ.xxx, 1);
#endif
}