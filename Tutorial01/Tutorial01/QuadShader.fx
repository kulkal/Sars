Texture2D<float4> texWorldNormal : register( t0 );
Texture2D<float4> texDepth : register( t1 );
SamplerState samLinear : register( s0 );

cbuffer ConstantBuffer : register( b0 )
{
	matrix View;
	matrix Projection;
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
    //return  Projection._43 / (DeviceZ - Projection._33);
	return  DeviceZ;
#endif
}