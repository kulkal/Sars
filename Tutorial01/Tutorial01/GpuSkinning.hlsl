
#if GPUSKINNING

Buffer<float4> BoneMatrices;

#define MAX_BONELINK 4
float4x4 CalcBoneMatrix(uint4 Bones, float4 Weights)
{
	float4x4 TotalMat = (float4x4)0;

	for(int i=0;i<MAX_BONELINK;i++)
	{
		uint iBone = Bones[i] * 4;
		float4 row1 = BoneMatrices.Load( iBone );
		float4 row2 = BoneMatrices.Load( iBone + 1 );
		float4 row3 = BoneMatrices.Load( iBone + 2 );
		float4 row4 = BoneMatrices.Load( iBone + 3 );
		float4x4 Mat = float4x4( row1, row2, row3, row4 );
		
		TotalMat += Mat* Weights[i];
	}

	return TotalMat;
}
#endif