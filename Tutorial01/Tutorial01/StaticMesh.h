#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <fbxsdk.h>
#include <string>
#include <vector>

#include "ShaderRes.h"
#include "baseobject.h"
#include "FbxFileImporter.h"

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
	ENumTexCoord _NumTexCoord;
	XMFLOAT3* _PositionArray;
	XMFLOAT3* _NormalArray;
	XMFLOAT2* _TexCoordArray;
	WORD*  _IndiceArray;

	int _NumTriangle;
	int _NumVertex;

	ID3D11Buffer*           _VertexBuffer;
	ID3D11Buffer*           _IndexBuffer;
	unsigned int _VertexStride;

	class SubMesh
	{
	public:
		int _TriangleCount;
		int _IndexOffset;
	};

	std::vector<SubMesh*> _SubMeshArray;
public:

	bool ImportFromFbxMesh(FbxMesh* Mesh, FbxFileImporter* Importer);

	StaticMesh(void);
	virtual ~StaticMesh(void);
};

