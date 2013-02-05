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
	std::vector<XMFLOAT3> _PositionArray;
	std::vector<XMFLOAT3> _NormalArray;
	std::vector<XMFLOAT2> _TexCoordArray;
	std::vector<DWORD> _IndiceArray;

	int _NumTexCoord;
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
		SubMesh()
		{
			_TriangleCount = 0;
			_IndexOffset = 0;
		}
	};

	std::vector<SubMesh*> _SubMeshArray;
public:

	bool ImportFromFbxMesh(FbxMesh* Mesh, FbxFileImporter* Importer);


	StaticMesh(void);
	virtual ~StaticMesh(void);
};

