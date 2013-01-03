#pragma once;

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <fbxsdk.h>
#include <string>
#include <vector>

struct SkeletonJoint
{
	std::string _Name;
	int			_ParentIndex;
	XMFLOAT4X4	_InvRefPose;
};
struct Skeleton
{
	int				_JointCount;
	SkeletonJoint*	_Joints;
};

struct JointPose
{
	XMFLOAT4	_Rot;
	XMFLOAT3	_Trans;
	float		_Scale;
};

struct SkeletonPose
{
	JointPose*	_LocalPoseArray;
};