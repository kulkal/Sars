#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <fbxsdk.h>
#include <string>
#include <vector>

#include "ShaderRes.h"
#include "FbxFileImporter.h"

#include "baseobject.h"

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

class SkeletalMesh :
	public BaseObject
{
public:
	XMFLOAT3* _PositionArray;
	XMFLOAT3* _NormalArray;
	XMFLOAT2* _TexCoordArray;

	WORD*  _IndiceArray;

	DWORD* _WeightArray;
	DWORD* _BoneIndexArray;

	ENumTexCoord _NumTexCoord;
	int _NumTriangle;
	int _NumVertex;

	ID3D11Buffer*           _VertexBuffer;
	ID3D11Buffer*           _IndexBuffer;
	ID3D11Texture2D*		_BoneMatrices;

	unsigned int _VertexStride;
public:
	SkeletalMesh(void);
	virtual ~SkeletalMesh(void);
};

