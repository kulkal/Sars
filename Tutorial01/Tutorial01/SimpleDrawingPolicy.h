#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include "DrawingPolicy.h"
class SimpleDrawingPolicy :
	public DrawingPolicy
{
	ID3D11Buffer*           ConstantBuffer;

	// temp
public:
	XMFLOAT4 vLightDirs[2];
	XMFLOAT4 vLightColors[2];
public:

	virtual void DrawStaticMesh(StaticMesh* pMesh);

	SimpleDrawingPolicy(void);
	virtual ~SimpleDrawingPolicy(void);
};

