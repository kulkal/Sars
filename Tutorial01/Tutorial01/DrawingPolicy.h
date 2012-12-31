#pragma once

#include <d3d11.h>

class StaticMesh;

class DrawingPolicy
{
protected:
	ID3D11InputLayout*      VertexLayout;
	ID3D11VertexShader*     VertexShader;
	ID3D11PixelShader*      PixelShader;
public:
	virtual void DrawStaticMesh(StaticMesh* pMesh) = 0;
	DrawingPolicy(void);
	virtual ~DrawingPolicy(void);
};

