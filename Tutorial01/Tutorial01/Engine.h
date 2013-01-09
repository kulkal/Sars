#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <d3dcompiler.h>

#include "Util.h"

class SimpleDrawingPolicy;
class LineBatcher;
class Engine
{
public:
	ID3D11Device*           _Device;
	ID3D11DeviceContext*    _ImmediateContext;

	SimpleDrawingPolicy* _SimpleDrawer;


	XMFLOAT4X4 _ViewMat;
	XMFLOAT4X4 _ProjectionMat;

	LineBatcher* _LineBatcher;

public:

	void InitDevice();

	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	Engine(void);
	virtual ~Engine(void);
};

extern Engine* GEngine;
