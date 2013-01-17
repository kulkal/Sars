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
	float n = 10;
	float f = 1000;
    //float LInearZ = (2.0 * n) / (f + n - DeviceZ * (f - n));
	//float LInearZ = -Projection._34/(DeviceZ +Projection._33);
	float LInearZ = 1.010101/(-DeviceZ -10.10101);

	return  float4(LInearZ.xxx, 1);
#endif
}