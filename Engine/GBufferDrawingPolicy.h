#pragma once
#include "drawingpolicy.h"
#include "MeshVertexShader.h"

class GBufferVertexShader :
	public MeshVertexShader
{
public:
	GBufferVertexShader(char* szFileName, char* szFuncName)
		:MeshVertexShader(szFileName, szFuncName)
	{
	}
	virtual ~GBufferVertexShader(void){}
};


class GBufferDrawingPolicy :
	public DrawingPolicy
{
	ID3D11Buffer*           ConstantBuffer;
	GBufferVertexShader* _VertexShader;
public:
	virtual void DrawStaticMesh(StaticMesh* pMesh, XMMATRIX& ViewMat, XMMATRIX& ProjectionMat);
	virtual void DrawSkeletalMeshData(SkeletalMeshRenderData* pRenderData, XMMATRIX& ViewMat, XMMATRIX& ProjectionMat);
	
	GBufferDrawingPolicy(void);
	virtual ~GBufferDrawingPolicy(void);
};

