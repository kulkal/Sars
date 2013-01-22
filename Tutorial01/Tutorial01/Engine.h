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
class Texture2D;
class TextureDepth2D;

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

	Texture2D*				_FrameBufferTexture;
	Texture2D*				_SceneColorTexture;
	Texture2D*				_LitTexture;
	Texture2D*				_WorldNormalTexture;
	TextureDepth2D*			_DepthTexture;

	UINT _Width;
	UINT _Height;

	SimpleDrawingPolicy* _SimpleDrawer;
	GBufferDrawingPolicy* _GBufferDrawer;


	XMFLOAT4X4 _ViewMat;
	XMFLOAT4X4 _ProjectionMat;
	float		_Near;
	float		_Far;

	// debug line draw
	LineBatcher* _LineBatcher;

	// timer
	float _TimeSeconds;
	float _DeltaSeconds;

	LARGE_INTEGER _PrevTime;
	LARGE_INTEGER _Freq;

	// full sqreen quad
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

	ID3D11PixelShader*			_CombineLitPS;


	// Depth Stencil States
	enum EDepthStencilState
	{
		DS_GBUFFER_PASS,
		DS_LIGHTING_PASS,
		SIZE_DEPTHSTENCILSTATE,
	};
	struct DepthStencilStateData
	{
		ID3D11DepthStencilState* DSS;
		UINT StencilRef;
		DepthStencilStateData()
		{
			DSS = NULL;
			StencilRef = 0;
		}
	};
	std::vector<DepthStencilStateData> _DepthStencilStateArray;

	// Blend States
	enum EBlendState{
		BS_NORMAL, BS_LIGHTING,
		SIZE_BLENDSTATE,
	};
	struct BlendStateData
	{
		ID3D11BlendState* BS;
		float BlendFactor[8];
		unsigned int SampleMask;
		BlendStateData()
		{
			BS = NULL;
			for(int i=0;i<8;i++)
				BlendFactor[i] = 1.f;
			SampleMask = 0xffffffff;
		}
	};
	std::vector<BlendStateData> _BlendStateArray;


	bool _VisualizeWorldNormal;
	bool _VisualizeDepth;
public:
	void Tick();
	void InitDevice();
	void BeginRendering();
	void EndRendering();
	void InitDeviceStates();
	void SetBlendState(EBlendState eBS);
	void SetDepthStencilState(EDepthStencilState eDSS);

	void StartRenderingFrameBuffer();

	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	ID3D11PixelShader* CreatePixelShaderSimple( char* szFileName, D3D10_SHADER_MACRO* pDefines = NULL);
	void DrawFullScreenQuad11( ID3D11PixelShader* pPS, UINT Width, UINT Height, UINT TopLeftX = 0, UINT TopLeftY = 0);

	Engine(void);
	virtual ~Engine(void);
};

extern Engine* GEngine;
