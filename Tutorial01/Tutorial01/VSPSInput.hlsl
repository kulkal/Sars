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