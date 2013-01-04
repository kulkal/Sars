#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <fbxsdk.h>
#include <string>
#include <vector>

#include "ShaderRes.h"
#include "FbxFileImporter.h"
#include "Skeleton.h"

#include "baseobject.h"

#define MAX_BONELINKE 4
struct NormalVertexGpuSkin
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	unsigned int Weights[MAX_BONELINKE];
	unsigned int Bones[MAX_BONELINKE];
};

struct NormalTexVertexGpuSkin
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	unsigned int Weights[MAX_BONELINKE];
	unsigned int Bones[MAX_BONELINKE];
};

struct SkinInfo
{
	float			Weights[MAX_BONELINKE];
	unsigned int	Bones[MAX_BONELINKE];
};



class SkeletalMesh :
	public BaseObject
{
public:
	XMFLOAT3* _PositionArray;
	XMFLOAT3* _NormalArray;
	XMFLOAT2* _TexCoordArray;

	WORD*  _IndiceArray;

	SkinInfo* _SkinInfoArray;

	

	int _NumTexCoord;
	int _NumTriangle;
	int _NumVertex;

	ID3D11Buffer*           _VertexBuffer;
	ID3D11Buffer*           _IndexBuffer;
	ID3D11Buffer*		_BoneMatricesBuffer;

	unsigned int _VertexStride;

	class SubMesh
	{
	public:
		int _TriangleCount;
		int _IndexOffset;
	};

	std::vector<SubMesh*> _SubMeshArray;

	// linked bone data.
	int _NumBone;
	XMFLOAT4X4* _BoneMatrices;
	XMFLOAT4X4* _BoneWorld;

	int* _RequiredBoneArray;
	
	Skeleton*	_Skeleton;
	SkeletonPose* _Pose;
public:
	bool ImportFromFbxMesh(FbxMesh* Mesh, FbxFileImporter* Importer);

	void UpdateBoneMatrices();

	SkeletalMesh(void);
	virtual ~SkeletalMesh(void);
};

