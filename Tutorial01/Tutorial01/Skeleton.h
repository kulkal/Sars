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

	SkeletonJoint()
		:
		_ParentIndex(-1)
	{
	}

};
class Skeleton
{
public:
	int				_JointCount;
	std::vector<SkeletonJoint> _Joints;
	Skeleton()
		:_JointCount(0)
	{
	}
	~Skeleton()
	{
	}
};

struct JointPose
{
	XMFLOAT4	_Rot;
	XMFLOAT3	_Trans;
	XMFLOAT3	_Scale;
};

class SkeletonPose
{
public:
	std::vector<JointPose> _LocalPoseArray;

	SkeletonPose()
	{
	}
	~SkeletonPose()
	{
	}
};