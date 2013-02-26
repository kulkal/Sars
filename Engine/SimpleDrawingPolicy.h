#pragma once

#include "DrawingPolicy.h"
#include "ShaderRes.h"


class SimpleDrawingPolicy :
	public DrawingPolicy
{
	ID3D11Buffer*           ConstantBuffer;

public:
	XMFLOAT4 vLightDirs[2];
	XMFLOAT4 vLightColors[2];
public:


	virtual void DrawStaticMesh(StaticMesh* pMesh, XMMATRIX& ViewMat, XMMATRIX& ProjectionMat);
	virtual void DrawSkeletalMeshData(SkeletalMeshRenderData* pRenderData, XMMATRIX& ViewMat, XMMATRIX& ProjectionMat);

	SimpleDrawingPolicy(void);
	virtual ~SimpleDrawingPolicy(void);
};

