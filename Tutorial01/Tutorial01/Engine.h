#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <d3dcompiler.h>
#include <vector>

#include "Util.h"
#include "OutputDebug.h"

class SimpleDrawingPolicy;
class GBufferDrawingPolicy;
class LineBatcher;
class Engine
{
public:
	HWND                    _hWnd;
	
	// d3d11
	ID3D11Device*           _Device;
	ID3D11DeviceContext*    _ImmediateContext;
	D3D_DRIVER_TYPE         _DriverType ;
	D3D_FEATURE_LEVEL       _FeatureLevel;
	IDXGISwapChain*         _SwapChain;

	ID3D11Texture2D*		_BackBuffer;
	ID3D11RenderTargetView* _RenderTargetView;

	ID3D11Texture2D*		_WorldNormalBuffer;
	ID3D11RenderTargetView* _WorldNormalView;

	std::vector<ID3D11RenderTargetView*> _RTViewArray;


	ID3D11Texture2D*        _DepthStencilTexture;
	ID3D11DepthStencilView* _DepthStencilView;

	UINT _Width;
	UINT _Height;

	SimpleDrawingPolicy* _SimpleDrawer;
	GBufferDrawingPolicy* _GBufferDrawer;


	XMFLOAT4X4 _ViewMat;
	XMFLOAT4X4 _ProjectionMat;

	// debug line draw
	LineBatcher* _LineBatcher;

	// timer
	float _TimeSeconds;
	float _DeltaSeconds;

	LARGE_INTEGER _PrevTime;
	LARGE_INTEGER _Freq;

	
public:
	void Tick();
	void InitDevice();
	void BeginRendering();
	void EndRendering();

	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

	Engine(void);
	virtual ~Engine(void);
};

extern Engine* GEngine;
