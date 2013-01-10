#pragma once
#include "basecomponent.h"

class SkeletalMesh;

class SkeletalMeshComponent :
	public BaseComponent
{
	ID3D11Buffer*				_VertexBuffer;
	ID3D11Buffer*				_IndexBuffer;
	ID3D11Buffer*				_BoneMatricesBuffer;
	ID3D11ShaderResourceView*	_BoneMatricesBufferRV;

	XMFLOAT4X4*					_BoneMatrices;
	XMFLOAT4X4*					_BoneWorld;

	SkeletalMesh*				_SkeletalMesh;
public:
	void UpdateBoneMatrices()

	void SetSkeletalMesh(SkeletalMesh* SkeletalMesh);
	void SetSkeleton(Skeleton* Skeleton);

	SkeletalMeshComponent(void);
	virtual ~SkeletalMeshComponent(void);
};

