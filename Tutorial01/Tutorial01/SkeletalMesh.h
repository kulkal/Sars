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

#define MAX_BONELINK 4
struct NormalVertexGpuSkin
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	unsigned int Weights;
	unsigned int Bones;
};

struct NormalTexVertexGpuSkin
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
	unsigned int Weights;
	unsigned int Bones;
};

struct SkinInfo
{
	float			Weights[MAX_BONELINK];
	unsigned int	Bones[MAX_BONELINK];
};


class SkeletalMesh :
	public BaseObject
{
	friend class SkeletalMeshComponent;
public:

	std::vector<XMFLOAT3> _PositionArray;
	std::vector<XMFLOAT3> _NormalArray;
	std::vector<XMFLOAT2> _TexCoordArray;
	std::vector<WORD> _IndiceArray;

	std::vector<SkinInfo> _SkinInfoArray;

	int _NumTexCoord;
	int _NumTriangle;
	int _NumVertex;

	ID3D11Buffer*				_VertexBuffer;
	ID3D11Buffer*				_IndexBuffer;
	ID3D11Buffer*				_BoneMatricesBuffer;
	ID3D11ShaderResourceView*	_BoneMatricesBufferRV;
	unsigned int _VertexStride;

	class SubMesh
	{
	public:
		int _TriangleCount;
		int _IndexOffset;
	};

	std::vector<SubMesh*> _SubMeshArray;

	int _NumBone;

	std::vector<int> _RequiredBoneArray;
	
	Skeleton*	_Skeleton;
	SkeletonPose* _Pose;
public:
	bool ImportFromFbxMesh(FbxMesh* Mesh, FbxFileImporter* Importer);

	SkeletalMesh(void);
	virtual ~SkeletalMesh(void);
};

