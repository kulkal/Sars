#pragma once

#include "DrawingPolicy.h"
#include "ShaderRes.h"


class SimpleDrawingPolicy :
	public DrawingPolicy
{
	ID3D11Buffer*           ConstantBuffer;
	ID3D11RasterizerState*	 RS;


public:
	XMFLOAT4 vLightDirs[2];
	XMFLOAT4 vLightColors[2];
public:


	virtual void DrawStaticMesh(StaticMesh* pMesh);
	virtual void DrawSkeletalMesh(SkeletalMesh* pMesh);


	SimpleDrawingPolicy(void);
	virtual ~SimpleDrawingPolicy(void);
};

