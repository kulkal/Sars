#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <fbxsdk.h>
#include <string>
#include <vector>

#include "baseobject.h"

struct NormalVertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
};

struct NormalTexVertex
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

class StaticMesh :
	public BaseObject
{
public:
	XMFLOAT3* PositionArray;
	XMFLOAT3* NormalArray;
	XMFLOAT2* TexCoordArray;
	WORD*  IndiceArray;

	int NumTriangle;
	int NumVertex;

	ID3D11Buffer*           VertexBuffer;
	ID3D11Buffer*           IndexBuffer;

	class SubMesh
	{
	public:
		int TriangleCount;
		int IndexOffset;
	};

	std::vector<SubMesh*> SubMeshArray;
public:

	bool ImportFromFbxMesh(FbxMesh* pMesh);

	StaticMesh(void);
	virtual ~StaticMesh(void);
};

