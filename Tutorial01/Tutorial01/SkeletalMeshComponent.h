#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#include "basecomponent.h"
#include "SkeletalMesh.h"
class SkeletalMesh;
class Skeleton;

class SkeletalMeshRenderData;

class SkeletalMeshComponent :
	public BaseComponent
{
public:
	std::vector<SkeletalMeshRenderData*> _RenderDataArray;

	friend class 	SkeletalMeshRenderData;

	std::vector<SkeletalMesh*> _SkeletalMeshArray;

	XMFLOAT4X4* _BoneWorld;

	Skeleton*	_Skeleton;
	SkeletonPose* _Pose;
public:
	void UpdateBoneMatrices();

	void AddSkeletalMesh(SkeletalMesh* InSkeletalMesh);
	void SetSkeleton(Skeleton* Skeleton);
	void SetCurrentPose(SkeletonPose* Pose);

	SkeletalMeshComponent(void);
	virtual ~SkeletalMeshComponent(void);
};

