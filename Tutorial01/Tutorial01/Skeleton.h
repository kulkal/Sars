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
		:_Name(""),
		_ParentIndex(-1)
	{
	}

};
class Skeleton
{
public:
	int				_JointCount;
	SkeletonJoint*	_Joints;
	Skeleton()
		:_JointCount(0),
		_Joints(NULL)
	{
	}
	~Skeleton()
	{
		if(_Joints) delete[] _Joints;
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
	JointPose*	_LocalPoseArray;

	SkeletonPose()
		:_LocalPoseArray(NULL)
	{
	}
	~SkeletonPose()
	{
				if(_LocalPoseArray) delete[] _LocalPoseArray;


	}
};