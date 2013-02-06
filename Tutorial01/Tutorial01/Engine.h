#pragma once

#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <d3dcompiler.h>
#include <vector>

#include "Util.h"
#include "OutputDebug.h"
#include "Skeleton.h"

class SimpleDrawingPolicy;
class GBufferDrawingPolicy;
class LineBatcher;
class Texture2D;
class TextureDepth2D;

class StaticMesh;
class SkeletalMesh;
class SkeletalMeshComponent;
class AnimationClip;
class LightComponent;
class DirectionalLightComponent;
class StaticMeshComponent;
class Camera;
class Input;

struct ShadowCascadeInfo
{
	bool				_bEnabled;
	float				_ViewNear;
	float				_ViewFar;
	TextureDepth2D*		_ShadowDepthTexture;
	XMFLOAT4X4			_ShadowViewMat;
	XMFLOAT4X4			_ShadowProjectionMat;
	float				_TextureSize;
	ShadowCascadeInfo(float ViewNear, float ViewFar, float TextureSize);
	~ShadowCascadeInfo();
};

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
	Texture2D*				_DeferredShadowTexture;

	Texture2D*				_WorldNormalTexture;
	TextureDepth2D*			_DepthTexture;
	//TextureDepth2D*			_ShadowDepthTexture;
	//UINT					_ShadowMapSize;

	std::vector<ShadowCascadeInfo*> _CascadeArray;


	float _Width;
	float _Height;

	SimpleDrawingPolicy* _SimpleDrawer;
	GBufferDrawingPolicy* _GBufferDrawer;


	XMFLOAT4X4 _ViewMat;
	XMFLOAT4X4 _ProjectionMat;

	//XMFLOAT4X4 _SunShadowMat;
	//XMFLOAT4X4 _SunShadowProjectionMat;

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

	ID3D11PixelShader*			_DeferredShadowPS;
	ID3D11Buffer*				_DeferredShadowPSCB;

	bool _VisualizeWorldNormal;
	bool _VisualizeDepth;

	StaticMeshComponent* _StaticMeshComponent;
	std::vector<StaticMesh*> _StaticMeshArray;
	std::vector<SkeletalMesh*> _SkeletalMeshArray;
	std::vector<AnimationClip*> _AnimClipArray;
	Skeleton* _GSkeleton;
	SkeletonPose* _GPose;
	SkeletalMeshComponent* _GSkeletalMeshComponent;
	ID3D11ShaderResourceView*           _TextureRV ;

	std::vector<LightComponent*> _LightCompArray;

	DirectionalLightComponent* _SunLight;

	Camera*		_CurrentCamera;

	Input*		_Input;
public:
	void Tick();
	void InitDevice();
	void BeginRendering();
	void Render();
	void EndRendering();

	void RenderShadowMap();
	void RenderDeferredShadow();
	
	float _GetTimeSeconds();	

	void StartRenderingFrameBuffer(bool bClearColor, bool bClearDepth, bool bReadOnlyDepth);
	void StartRenderingGBuffers();
	void StartRenderingLightingBuffer(bool bClear);

	HRESULT CompileShaderFromFile( WCHAR* szFileName, D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );
	ID3D11PixelShader* CreatePixelShaderSimple( char* szFileName, D3D10_SHADER_MACRO* pDefines = NULL);
	void DrawFullScreenQuad11( ID3D11PixelShader* pPS, float Width, float Height, float TopLeftX = 0, float TopLeftY = 0);

	Engine(void);
	virtual ~Engine(void);
};

extern Engine* GEngine;
