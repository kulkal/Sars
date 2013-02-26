#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#include "basecomponent.h"
#include "SkeletalMesh.h"
class SkeletalMesh;
class Skeleton;
class AnimationClip;
class AnimClipInstance;

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

	AnimClipInstance* _CurrentAnim;
public:
	void PlayAnim(AnimationClip* InClip, int InNumPlay = 0, float InRate = 1.f);
	void Tick(float DeltaSeconds);
	void UpdateBoneMatrices();

	void AddSkeletalMesh(SkeletalMesh* InSkeletalMesh);
	void SetSkeleton(Skeleton* Skeleton);
	void SetCurrentPose(SkeletonPose* Pose);
	void SetCurrentAnim(AnimationClip* InClip);

	SkeletalMeshComponent(void);
	virtual ~SkeletalMeshComponent(void);
};

