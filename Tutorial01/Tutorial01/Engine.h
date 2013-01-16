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

	ID3D11Texture2D*			_WorldNormalBuffer;
	ID3D11RenderTargetView*		 _WorldNormalView;
	ID3D11ShaderResourceView *	_WorldNormalRV;



	ID3D11Texture2D*        _DepthStencilTexture;
	ID3D11DepthStencilView* _DepthStencilView;
	ID3D11ShaderResourceView *	_DepthStencilSRV;
	ID3D11DepthStencilView *	_ReadOnlyDepthStencilView;


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

	// fullsqreen quad
	ID3D11Buffer*               g_pScreenQuadVB;
	ID3D11InputLayout*          g_pQuadLayout;
	ID3D11VertexShader*         g_pQuadVS;
	ID3D11PixelShader*			_VisNormalPS;

	ID3D11PixelShader*			_VisDpethPS;
	ID3D11Buffer*				_VisDpethPSCB;


	ID3D11PixelShader*			_DeferredDirPS;
	ID3D11Buffer*				_DeferredDirPSCB;

	ID3D11PixelShader*			_DeferredPointPS;
	ID3D11Buffer*				_DeferredPointPSCB;

	ID3D11DepthStencilState*	_DepthStateEnable;
	ID3D11DepthStencilState*	_DepthStateDisable;


	bool _VisualizeWorldNormal;
	bool _VisualizeDepth;
public:
	void Tick();
	void InitDevice();
	void BeginRendering();
	void EndRendering();

	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	ID3D11PixelShader* CreatePixelShaderSimple( char* szFileName, D3D10_SHADER_MACRO* pDefines = NULL);
	void DrawFullScreenQuad11( ID3D11PixelShader* pPS, UINT Width, UINT Height, UINT TopLeftX = 0, UINT TopLeftY = 0);

	Engine(void);
	virtual ~Engine(void);
};

extern Engine* GEngine;
