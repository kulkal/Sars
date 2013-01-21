#pragma once
#include "drawingpolicy.h"
class GBufferDrawingPolicy :
	public DrawingPolicy
{
	ID3D11Buffer*           ConstantBuffer;
	ID3D11RasterizerState*	 RS;
	ID3D11SamplerState*		_SamplerLinear;
public:
	virtual void DrawStaticMesh(StaticMesh* pMesh);
	virtual void DrawSkeletalMeshData(SkeletalMeshRenderData* pRenderData);
	
	GBufferDrawingPolicy(void);
	virtual ~GBufferDrawingPolicy(void);
};

