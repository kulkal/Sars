#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

class SkeletalMesh;
class SkeletalMeshComponent;
class SkeletalMeshRenderData
{
public:

	ID3D11Buffer*				_BoneMatricesBuffer;
	ID3D11ShaderResourceView*	_BoneMatricesBufferRV;
	XMFLOAT4X4* _BoneMatrices;

	SkeletalMesh* _SkeletalMesh;
	SkeletalMeshComponent* _SkeletalMeshComponent;

public:
	SkeletalMeshRenderData(SkeletalMesh* InSkeletalMesh, SkeletalMeshComponent* InSkeletalMeshComponent );
	~SkeletalMeshRenderData();

	void UpdateBoneMatrices();
};