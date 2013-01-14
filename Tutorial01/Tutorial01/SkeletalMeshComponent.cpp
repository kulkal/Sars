#include <cassert>
#include "SkeletalMeshComponent.h"
#include "Engine.h"
#include "LineBatcher.h"
#include "SkeletalMeshRenderData.h"
#include "AnimClipInstance.h"


SkeletalMeshComponent::SkeletalMeshComponent(void)
	:_BoneWorld(NULL)
	,_Skeleton(NULL)
	,_Pose(NULL)
	,_CurrentAnim(NULL)
{
}


SkeletalMeshComponent::~SkeletalMeshComponent(void)
{
	if(_BoneWorld) delete[] _BoneWorld;

	for(unsigned int i=0;i<_RenderDataArray.size();i++)
	{
		delete _RenderDataArray[i];
	}

}

void SkeletalMeshComponent::SetSkeleton(Skeleton* Skeleton)
{
	_Skeleton = Skeleton;

	if( _BoneWorld)
		delete _BoneWorld;
	_BoneWorld = new XMFLOAT4X4[_Skeleton->_JointCount];
}

void SkeletalMeshComponent::SetCurrentPose(SkeletonPose* Pose)
{
	_Pose = Pose;
}

void SkeletalMeshComponent::UpdateBoneMatrices()
{
	// debug draw ref pose line
	for(int i=0;i<_Skeleton->_JointCount;i++)
	{
		XMMATRIX MatParent;

		SkeletonJoint& RefPose = _Skeleton->_Joints[i];

		XMFLOAT4X4& RefInvF = _Skeleton->_Joints[i]._InvRefPose;
		XMVECTOR Det;
		XMMATRIX RefMat = XMMatrixInverse(&Det, XMLoadFloat4x4(&RefInvF));

		if(RefPose._ParentIndex < 0)
			continue;

		XMFLOAT4X4& RefInvParentF = _Skeleton->_Joints[RefPose._ParentIndex]._InvRefPose;
		XMMATRIX RefMatParent = XMMatrixInverse(&Det, XMLoadFloat4x4(&RefInvParentF));

		if( RefPose._ParentIndex >= 0)
			GEngine->_LineBatcher->AddLine(XMFLOAT3(RefMat._41, RefMat._42, RefMat._43), XMFLOAT3(RefMatParent._41, RefMatParent._42, RefMatParent._43), XMFLOAT3(0, 1, 0), XMFLOAT3(0, 0, 1));
	}

	// calc world bone
	for(int i=0;i<_Skeleton->_JointCount;i++)
	{
		SkeletonJoint& RefPose = _Skeleton->_Joints[i];
		JointPose& LocalPose = _Pose->_LocalPoseArray[i];
		XMMATRIX MatScale = XMMatrixScaling(LocalPose._Scale.x, LocalPose._Scale.y, LocalPose._Scale.z);
		XMVECTOR QuatVec = XMLoadFloat4(&LocalPose._Rot);
		XMMATRIX MatRot = XMMatrixRotationQuaternion(QuatVec);
		XMMATRIX MatTrans = XMMatrixTranslation(LocalPose._Trans.x, LocalPose._Trans.y, LocalPose._Trans.z);

		
		XMMATRIX MatBone;
		if(RefPose._ParentIndex < 0) // root
		{
			MatBone = XMMatrixMultiply(MatScale, MatRot);
			MatBone = XMMatrixMultiply(MatBone, MatTrans);

			XMStoreFloat4x4(&_BoneWorld[i], MatBone);
		}
		else
		{
			XMMATRIX MatParent;
			MatParent = XMLoadFloat4x4(&_BoneWorld[RefPose._ParentIndex]);

			MatBone = XMMatrixMultiply(MatScale, MatRot);
			MatBone = XMMatrixMultiply(MatBone, MatTrans);
			MatBone = XMMatrixMultiply(MatBone, MatParent);

			GEngine->_LineBatcher->AddLine(XMFLOAT3(MatParent._41, MatParent._42, MatParent._43), XMFLOAT3(MatBone._41, MatBone._42, MatBone._43), XMFLOAT3(1, 0, 0), XMFLOAT3(1, 0, 0));

			XMStoreFloat4x4(&_BoneWorld[i], MatBone);
		}
	}

	// ref inverse * bone world
	for(int i=0;i<_Skeleton->_JointCount;i++)
	{
		XMMATRIX RefInv;
		XMFLOAT4X4& RefInvF = _Skeleton->_Joints[i]._InvRefPose;
		RefInv = XMLoadFloat4x4(&RefInvF);

		XMMATRIX MatBone;
		MatBone = XMLoadFloat4x4(&_BoneWorld[i]);
		
		MatBone = XMMatrixMultiply(RefInv, MatBone);
		XMStoreFloat4x4(&_BoneWorld[i], MatBone);
	}

	for(unsigned int i=0;i<_RenderDataArray.size();i++)
	{
		SkeletalMeshRenderData* RenderData = _RenderDataArray[i];
		RenderData->UpdateBoneMatrices();
	}
}

void SkeletalMeshComponent::AddSkeletalMesh(SkeletalMesh* InSkeletalMesh)
{
	_SkeletalMeshArray.push_back(InSkeletalMesh);

	SkeletalMeshRenderData *RenderData = new SkeletalMeshRenderData(InSkeletalMesh, this);
	_RenderDataArray.push_back(RenderData);
}

void SkeletalMeshComponent::Tick( float DeltaSeconds )
{
	if(_CurrentAnim)
	{
		_CurrentAnim->Tick();
		_CurrentAnim->GetCurrentPose(*_Pose);
	}
	UpdateBoneMatrices();
}

void SkeletalMeshComponent::PlayAnim(AnimationClip* InClip, int InNumPlay, int InRate)
{
	_CurrentAnim = new AnimClipInstance(InClip);
	_CurrentAnim->Play(InNumPlay);
	_CurrentAnim->SetTimeScale(InRate);
}
