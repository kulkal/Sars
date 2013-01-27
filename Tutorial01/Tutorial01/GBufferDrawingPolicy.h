#pragma once
#include "drawingpolicy.h"
class GBufferDrawingPolicy :
	public DrawingPolicy
{
	ID3D11Buffer*           ConstantBuffer;
	ID3D11RasterizerState*	 RS;
public:
	virtual void DrawStaticMesh(StaticMesh* pMesh, XMFLOAT4X4& ViewMat, XMFLOAT4X4& ProjectionMat);
	virtual void DrawSkeletalMeshData(SkeletalMeshRenderData* pRenderData, XMFLOAT4X4& ViewMat, XMFLOAT4X4& ProjectionMat);
	
	GBufferDrawingPolicy(void);
	virtual ~GBufferDrawingPolicy(void);
};

