#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include "resource.h"

//
#include <xnamath.h>
#include <d3dcompiler.h>

class SimpleDrawingPolicy;

class Engine
{
public:
	ID3D11Device*           Device;
	ID3D11DeviceContext*    ImmediateContext;

	ID3D11InputLayout*      VertexLayout;
	ID3D11VertexShader*     VertexShader;
	ID3D11PixelShader*      PixelShader;

	SimpleDrawingPolicy* SimpleDrawer;

	XMMATRIX ViewMat;
	XMMATRIX ProjectionMat;

public:

	void InitDevice();

	HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	Engine(void);
	virtual ~Engine(void);
};

extern Engine* GEngine;
