#include <cassert>
#include "Engine.h"
#include "SimpleDrawingPolicy.h"
#include "LineBatcher.h"
#include "GBufferDrawingPolicy.h"
#include "Texture2D.h"
#include "TextureDepth2D.h"
#include "SkeletalMeshComponent.h"
#include "SkeletalMesh.h"
#include "DirectionalLightComponent.h"
#include "PointLightComponent.h"
#include "AnimationClip.h"
#include "StateManager.h"
#include "StaticMeshComponent.h"
#include "xnacollision.h"
#include "MathUtil.h"

struct SCREEN_VERTEX
{
	XMFLOAT4 pos;
	XMFLOAT2 tex;
};

struct DeferredShadowPSCBStruct
{
	XMMATRIX Projection;
	XMFLOAT4 ProjectionParams;
	XMFLOAT4 ViewportParams;
	XMMATRIX ShadowMatrix;
};

struct VisDepthPSCBStruct
{
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 ProjectionParams;
};

Engine* GEngine;
Engine::Engine(void)
	:_hWnd(NULL)
	,_Device(NULL)
	,_ImmediateContext(NULL)
	,_DriverType(D3D_DRIVER_TYPE_NULL)
	,_FeatureLevel(D3D_FEATURE_LEVEL_11_0)
	,_SwapChain(NULL)
	,_SimpleDrawer(NULL)
	,_LineBatcher(NULL)
	,_TimeSeconds(0.f)
	,_VisualizeWorldNormal(false)
	,_VisualizeDepth(false)
	,_DeferredDirPS(NULL)
	,_DeferredDirPSCB(NULL)
	,_DeferredPointPS(NULL)
	,_DeferredPointPSCB(NULL)
	,_DeferredShadowPS(NULL)
	,_DeferredShadowPSCB(NULL)
	,_VisNormalPS(NULL)
	,_VisDpethPS(NULL)
	,_VisDpethPSCB(NULL)
	,_WorldNormalTexture(NULL)
	,_DepthTexture(NULL)
	,_FrameBufferTexture(NULL)
	,_SceneColorTexture(NULL)
	,_LitTexture(NULL)
	,_CombineLitPS(NULL)
	,_TextureRV(NULL)
	,_ShadowDepthTexture(NULL)
	,_DeferredShadowTexture(NULL)
	,_StaticMeshComponent(NULL)
	
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	QueryPerformanceFrequency(&_Freq);
	QueryPerformanceCounter(&_PrevTime);
	_Near = 10.f;
	_Far = 1000.f;
	_ShadowMapSize = 512;
	//_CrtSetBreakAlloc(2486860);
}

Engine::~Engine(void)
{
	if( _ImmediateContext ) _ImmediateContext->ClearState();

	if( _SwapChain ) _SwapChain->Release();
	if( _ImmediateContext ) _ImmediateContext->Release();
	if( _Device ) _Device->Release();

	if(_VisNormalPS) _VisNormalPS->Release();
	if(_VisDpethPS) _VisDpethPS->Release();
	if(_VisDpethPSCB)_VisDpethPSCB->Release();
	
	if(g_pQuadVS) g_pQuadVS->Release();
	if(g_pScreenQuadVB) g_pScreenQuadVB->Release();
	if(g_pQuadLayout) g_pQuadLayout->Release();
	if(_DeferredDirPS) _DeferredDirPS->Release();
	if(_DeferredDirPSCB) _DeferredDirPSCB->Release();
	if(_DeferredPointPS) _DeferredPointPS->Release();
	if(_DeferredPointPSCB) _DeferredPointPSCB->Release();
	if(_DeferredShadowPS) _DeferredShadowPS->Release();
	if(_DeferredShadowPSCB) _DeferredShadowPSCB->Release();


	if(_SimpleDrawer) delete _SimpleDrawer;
	if(_GBufferDrawer) delete _GBufferDrawer;

	if(_LineBatcher) delete _LineBatcher;

	if(_WorldNormalTexture) delete _WorldNormalTexture;
	if(_DepthTexture) delete _DepthTexture;
	if(_FrameBufferTexture) delete _FrameBufferTexture;
	if(_SceneColorTexture) delete _SceneColorTexture;
	if(_LitTexture) delete _LitTexture;
	if(_ShadowDepthTexture) delete _ShadowDepthTexture;
	if(_DeferredShadowTexture) delete _DeferredShadowTexture;


	if(_CombineLitPS) _CombineLitPS->Release();

	

	if(_GSkeleton) delete _GSkeleton;
	if(_GPose) delete _GPose;
	if(_GSkeletalMeshComponent) delete _GSkeletalMeshComponent;
	if(_StaticMeshComponent) delete _StaticMeshComponent;

	for(unsigned int i=0;i<_StaticMeshArray.size();i++)
	{
		StaticMesh* Mesh = _StaticMeshArray[i];
		delete Mesh;
	}

	for(unsigned int i=0;i<_SkeletalMeshArray.size();i++)
	{
		SkeletalMesh* Mesh = _SkeletalMeshArray[i];
		delete Mesh;
	}

	for(unsigned int i=0;i<_AnimClipArray.size();i++)
	{
		AnimationClip* Clip = _AnimClipArray[i];
		delete Clip;
	}

	for(unsigned int i=0;i<_LightCompArray.size();i++)
	{
		LightComponent* Light = _LightCompArray[i];
		delete Light;
	}

	if( _TextureRV ) _TextureRV->Release();

	if(GStateManager) delete GStateManager;
}

void Engine::InitDevice()
{
	HRESULT hr;
	RECT rc;
	GetClientRect( _hWnd, &rc );
	_Width = rc.right - rc.left;
	_Height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE( driverTypes );

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE( featureLevels );

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _Width;
	sd.BufferDesc.Height = _Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
	{
		_DriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain( NULL, _DriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &_SwapChain, &_Device, &_FeatureLevel, &_ImmediateContext );
		if( SUCCEEDED( hr ) )
			break;
	}


	if( FAILED( hr ) )
		assert(false);

	// Create a render target view
	ID3D11Texture2D*		BackBuffer;
	hr = _SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&BackBuffer );
	if( FAILED( hr ) )
		assert(false);

	_FrameBufferTexture = new Texture2D(BackBuffer, true);

	// scene color
	CD3D11_TEXTURE2D_DESC DescSceneColorTex(DXGI_FORMAT_R16G16B16A16_FLOAT, _Width, _Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescSceneColorSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescSceneColorTex.Format, 0,  DescSceneColorTex.MipLevels);
	_SceneColorTexture = new Texture2D(DescSceneColorTex, DescSceneColorSRV, true);

	// lit
	CD3D11_TEXTURE2D_DESC DescLitTex(DXGI_FORMAT_R16G16B16A16_FLOAT, _Width, _Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescLitSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescSceneColorTex.Format, 0, DescLitTex.MipLevels);
	_LitTexture = new Texture2D(DescLitTex, DescLitSRV, true);

	// world normal
	CD3D11_TEXTURE2D_DESC DescWordNormalTex(DXGI_FORMAT_R16G16B16A16_FLOAT, _Width, _Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescWorldNormalSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescWordNormalTex.Format);
	_WorldNormalTexture = new Texture2D(DescWordNormalTex, DescWorldNormalSRV, true);

	// depth stencil texture
	CD3D11_TEXTURE2D_DESC DescDepthTex(DXGI_FORMAT_R24G8_TYPELESS, _Width, _Height, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_DEPTH_STENCIL_VIEW_DESC  DescDSV(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT, 0, 0, 0,0) ;
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescDepthSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	_DepthTexture = new TextureDepth2D(DescDepthTex, DescDSV, DescDepthSRV);

	CD3D11_TEXTURE2D_DESC ShadowDescDepthTex(DXGI_FORMAT_R24G8_TYPELESS, _ShadowMapSize, _ShadowMapSize, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_DEPTH_STENCIL_VIEW_DESC  ShadowDescDSV(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT, 0, 0, 0,0) ;
	CD3D11_SHADER_RESOURCE_VIEW_DESC ShadowDescDepthSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	_ShadowDepthTexture = new TextureDepth2D(ShadowDescDepthTex, ShadowDescDSV, ShadowDescDepthSRV);

	// shadow result
	CD3D11_TEXTURE2D_DESC DescShadowTex(DXGI_FORMAT_R8G8B8A8_UNORM, _Width, _Height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescShadowSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescShadowTex.Format, 0, DescShadowTex.MipLevels);
	_DeferredShadowTexture = new Texture2D(DescShadowTex, DescShadowSRV, true);
	

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_Width;
	vp.Height = (FLOAT)_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_ImmediateContext->RSSetViewports( 1, &vp );

	// prepare resources for full screen quad
	

	SCREEN_VERTEX svQuad[4];
	svQuad[0].pos = XMFLOAT4( -1.0f, 1.0f, 0.0f, 1.0f );
	svQuad[0].tex = XMFLOAT2( 0.0f, 0.0f );
	svQuad[1].pos = XMFLOAT4( -1.0f, -1.0f, 0.0f, 1.0f );
	svQuad[1].tex = XMFLOAT2( 0.0f, 1.0f );
	svQuad[2].pos = XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f );
	svQuad[2].tex = XMFLOAT2( 1.0f, 0.0f );
	svQuad[3].pos = XMFLOAT4( 1.0f, -1.0f, 0.0f, 1.0f );
	svQuad[3].tex = XMFLOAT2( 1.0f, 1.0f );
	D3D11_BUFFER_DESC vbdesc =
	{
		4 * sizeof( SCREEN_VERTEX ),
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_VERTEX_BUFFER,
		0,
		0
	};
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = svQuad;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;
	_Device->CreateBuffer( &vbdesc, &InitData, &g_pScreenQuadVB ) ;

	ID3DBlob* pBlob = NULL;
	CompileShaderFromFile( L"QuadShader.fx", NULL, "QuadVS", "vs_4_0", &pBlob ) ;
	hr = _Device->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &g_pQuadVS ) ;
	if( FAILED( hr ) )
		assert(false);
	SetD3DResourceDebugName("QuadVS", g_pQuadVS);

	const D3D11_INPUT_ELEMENT_DESC quadlayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = _Device->CreateInputLayout( quadlayout, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &g_pQuadLayout ) ;
	if( FAILED( hr ) )
		assert(false);
	if(pBlob) pBlob->Release();
	SetD3DResourceDebugName("QuadLayout", g_pQuadLayout);

	D3D10_SHADER_MACRO DefinesVisNormal[] = {{"VIS_NORMAL", "1"},{0, 0} };
	
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"QuadShader.fx", DefinesVisNormal, "PS", "ps_4_0", &pPSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	hr = _Device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &_VisNormalPS );
	pPSBlob->Release();
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("_VisNormalPS", _VisNormalPS);

	D3D10_SHADER_MACRO DefinesVisDepth[] = {{"VIS_DEPTH", "1"},{0, 0} };
	
	pPSBlob = NULL;
	hr = CompileShaderFromFile(L"QuadShader.fx", DefinesVisDepth, "PS", "ps_4_0", &pPSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	hr = _Device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &_VisDpethPS );
	pPSBlob->Release();
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("_VisDpethPS", _VisDpethPS);
	
	D3D11_BUFFER_DESC bdc;
	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(VisDepthPSCBStruct);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_VisDpethPSCB );
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("_VisDpethPSCB", _VisDpethPSCB);

	////////////////////////////////////////////
	// directional light
	_DeferredDirPS = CreatePixelShaderSimple("DeferredDirectional.fx");

	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(DeferredDirPSCBStruct);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_DeferredDirPSCB );
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("_DeferredDirPSCB", _DeferredDirPSCB);

	// point light
	_DeferredPointPS = CreatePixelShaderSimple("DeferredPoint.fx");

	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(DeferredPointPSCBStruct);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_DeferredPointPSCB );
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("_DeferredPointPSCB", _DeferredPointPSCB);
	
	_CombineLitPS = CreatePixelShaderSimple("ComblineShader.fx");

	// deferred shadow
	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(DeferredShadowPSCBStruct);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_DeferredShadowPSCB );
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("_DeferredSahdowtPSCB", _DeferredShadowPSCB);
	
	_DeferredShadowPS = CreatePixelShaderSimple("DeferredShadow.fx");

	/////////////
	_SimpleDrawer = new SimpleDrawingPolicy;
	_GBufferDrawer = new GBufferDrawingPolicy;
	_LineBatcher = new LineBatcher;
	_LineBatcher->InitDevice();



	// temp
	_GSkeletalMeshComponent = new SkeletalMeshComponent;
	_GSkeleton = new Skeleton;
	_GPose = new SkeletonPose;

	FbxFileImporter FbxImporterObj("humanoid.fbx");
	//FbxFileImporter FbxImporterObj("box_skin.fbx");
	//FbxImporterObj.ImportStaticMesh(StaticMeshArray);

	FbxImporterObj.ImportSkeletalMesh(_SkeletalMeshArray);

	for(unsigned int i=0;i<_SkeletalMeshArray.size();i++)
	{
		_GSkeletalMeshComponent->AddSkeletalMesh(_SkeletalMeshArray[i]);
	}
	FbxImporterObj.ImportSkeleton(&_GSkeleton, &_GPose);
	//FbxImporterObj.ImportAnimClip(_AnimClipArray);

	_GSkeletalMeshComponent->SetSkeleton(_GSkeleton);
	_GSkeletalMeshComponent->SetCurrentPose(_GPose);

	GEngine->Tick();

	//_GSkeletalMeshComponent->PlayAnim(_AnimClipArray[1], 0, 0.2f);

	FbxFileImporter FbxImporterObj2("other.fbx");
	FbxImporterObj2.ImportStaticMesh(_StaticMeshArray);

	_StaticMeshComponent = new StaticMeshComponent;
	for(int i=0;i<_StaticMeshArray.size();i++)
	{
		_StaticMeshComponent->AddStaticMesh(_StaticMeshArray[i]);
	}

	cout_debug("staticmesh aabb min: %f %f %f\n", _StaticMeshComponent->_AABBMin.x, _StaticMeshComponent->_AABBMin.y, _StaticMeshComponent->_AABBMin.z);
	cout_debug("staticmesh aabb max: %f %f %f\n", _StaticMeshComponent->_AABBMax.x, _StaticMeshComponent->_AABBMax.y, _StaticMeshComponent->_AABBMax.z);

	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile( GEngine->_Device, L"seafloor.dds", NULL, NULL, &_TextureRV, NULL );
	if( FAILED( hr ) )
		assert(false);

	GStateManager = new StateManager;
	GStateManager->Init();

	_SunLight = new DirectionalLightComponent(XMFLOAT4(0.2f, 0.f, 0.f, 1.F), XMFLOAT3(-1.0f, -1.f, -1.f) );
	_LightCompArray.push_back(_SunLight);

	PointLightComponent* PointLight1 = new PointLightComponent(XMFLOAT4( 0.f, 1.f, 0.f, 1.0f), XMFLOAT3(  0.f, 0, 100.f  ), 200.f);
	_LightCompArray.push_back(PointLight1);

	PointLightComponent* PointLight2 = new PointLightComponent(XMFLOAT4(  0.f, 0.f, 1.f, 1.0f), XMFLOAT3( 100.f, 50.f, 0.f ), 200.f);
	_LightCompArray.push_back(PointLight2);

}

ID3D11PixelShader* Engine::CreatePixelShaderSimple( char* szFileName, D3D10_SHADER_MACRO* pDefines)
{
	ID3D11PixelShader* PS;
	HRESULT hr;
	int nLen = strlen(szFileName)+1;
	wchar_t WFileName[1024];
	size_t RetSize;
	mbstowcs_s(&RetSize, WFileName, 1024, szFileName, nLen);

	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(WFileName, pDefines, "PS", "ps_4_0", &pPSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	hr = _Device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PS );
	pPSBlob->Release();
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName(szFileName, PS);

	return PS;
}

void Engine::DrawFullScreenQuad11( ID3D11PixelShader* pPS, UINT Width, UINT Height, UINT TopLeftX, UINT TopLeftY)
{
	// Save the old viewport
	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
	UINT nViewPorts = 1;
	_ImmediateContext->RSGetViewports( &nViewPorts, vpOld );

	// Setup the viewport to match the backbuffer
	D3D11_VIEWPORT vp;
	vp.Width = (float)Width;
	vp.Height = (float)Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = TopLeftX;
	vp.TopLeftY = TopLeftY;
	_ImmediateContext->RSSetViewports( 1, &vp );

	UINT strides = sizeof( SCREEN_VERTEX );
	UINT offsets = 0;
	ID3D11Buffer* pBuffers[1] = { g_pScreenQuadVB };

	_ImmediateContext->IASetInputLayout( g_pQuadLayout );
	_ImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
	_ImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	_ImmediateContext->VSSetShader( g_pQuadVS, NULL, 0 );
	_ImmediateContext->PSSetShader( pPS, NULL, 0 );
	_ImmediateContext->Draw( 4, 0 );

	// Restore the Old viewport
	_ImmediateContext->RSSetViewports( nViewPorts, vpOld );
}


HRESULT Engine::CompileShaderFromFile( WCHAR* szFileName, D3D10_SHADER_MACRO* pDefines, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile( szFileName, pDefines, NULL, szEntryPoint, szShaderModel, 
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		if( pErrorBlob != NULL )
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		if( pErrorBlob ) pErrorBlob->Release();
		return hr;
	}
	if( pErrorBlob ) pErrorBlob->Release();

	return S_OK;
}

void Engine::Tick()
{
	LARGE_INTEGER CurrentTime;

	QueryPerformanceCounter(&CurrentTime);
	_DeltaSeconds = (float)(CurrentTime.QuadPart - _PrevTime.QuadPart)/(float)_Freq.QuadPart;
	_TimeSeconds =	(float)(CurrentTime.QuadPart)/(float)_Freq.QuadPart;
	//cout_debug("delta seconds: %f\n", _DeltaSeconds);

	if(_GSkeletalMeshComponent) _GSkeletalMeshComponent->Tick(_DeltaSeconds);
	_PrevTime = CurrentTime;
}

void Engine::BeginRendering()
{
	_LineBatcher->BeginLine();

	// z pre pass?

	RenderShadowMap();
}


void Engine::Render()
{
	// g-buffer pass
	StartRenderingGBuffers();

	_ImmediateContext->PSSetShaderResources( 0, 1, &_TextureRV );

	// draw scene into g-buffer
	for(unsigned int i=0;i<_StaticMeshComponent->_StaticMeshArray.size();i++)
	{
		StaticMesh* Mesh = _StaticMeshComponent->_StaticMeshArray[i];
		_GBufferDrawer->DrawStaticMesh(Mesh, _ViewMat, _ProjectionMat);
	}

	if(_GSkeletalMeshComponent)
	{
		for(unsigned int i=0;i<_GSkeletalMeshComponent->_RenderDataArray.size();i++)
		{
			_GBufferDrawer->DrawSkeletalMeshData(_GSkeletalMeshComponent->_RenderDataArray[i], _ViewMat, _ProjectionMat);
		}
	}

	// render shadows to shadow result buffer
	RenderDeferredShadow();

	// lighting pass
	StartRenderingLightingBuffer(true);


	ID3D11ShaderResourceView* aSRV[3] = {_WorldNormalTexture->GetSRV(), _DepthTexture->GetSRV(), _DeferredShadowTexture->GetSRV()};
	_ImmediateContext->PSSetShaderResources( 0, 3, aSRV );

	SET_BLEND_STATE(BS_LIGHTING);
	SET_DEPTHSTENCIL_STATE(DS_LIGHTING_PASS);
	SET_PS_SAMPLER(0, SS_POINT);
	SET_PS_SAMPLER(1, SS_POINT);

	for(unsigned int i=0;i<_LightCompArray.size();i++)
	{
		LightComponent* Light = _LightCompArray[i];
		Light->RenderLightDeferred();
	}

	// combine pass
	SET_BLEND_STATE(BS_NORMAL);
	SET_PS_SAMPLER(0, SS_LINEAR);
	SET_PS_SAMPLER(1, SS_LINEAR);
	StartRenderingFrameBuffer(false, false, true);

	ID3D11ShaderResourceView* aSRVCombine[2] = {_SceneColorTexture->GetSRV(), _LitTexture->GetSRV()};
	_ImmediateContext->PSSetShaderResources( 0, 2, aSRVCombine );
	DrawFullScreenQuad11(_CombineLitPS, _Width, _Height);

}

void Engine::EndRendering()
{
	//_LineBatcher->Draw();
	
	ID3D11ShaderResourceView* aSRVVis[2] = {_WorldNormalTexture->GetSRV(), _DepthTexture->GetSRV()};
	_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis );

	_VisualizeDepth = true;
	if(_VisualizeDepth)
	{
		VisDepthPSCBStruct cbVisDepth;
		cbVisDepth.mView = XMMatrixTranspose( XMLoadFloat4x4( &_ViewMat ));
		cbVisDepth.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
		cbVisDepth.ProjectionParams.x = _Far/(_Far - _Near);
		cbVisDepth.ProjectionParams.y = _Near/(_Near - _Far);
		_ImmediateContext->UpdateSubresource( _VisDpethPSCB, 0, NULL, &cbVisDepth, 0, 0 );
		_ImmediateContext->PSSetConstantBuffers( 0, 1, &_VisDpethPSCB );

		DrawFullScreenQuad11(_VisDpethPS, _Width/4, _Height/4, _Width*0.75, 0);
	}

	_VisualizeWorldNormal = true;
	if(_VisualizeWorldNormal)
	{
		DrawFullScreenQuad11(_VisNormalPS, _Width/4, _Height/4);
	}

	bool _VisualizeShadow = false;
	if(_VisualizeShadow)
	{
		ID3D11ShaderResourceView* aSRVVis[1] = {_DeferredShadowTexture->GetSRV(), };
		_ImmediateContext->PSSetShaderResources( 0, 1, aSRVVis );
		DrawFullScreenQuad11(_VisNormalPS, _Width/2, _Height/2, _Width*0.5, _Height*0.5);
	}

	bool _VisualizeShadowMap = true;
	if(_VisualizeShadowMap)
	{
		ID3D11ShaderResourceView* aSRVVis[2] = {NULL, _ShadowDepthTexture->GetSRV()};
		_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis );

		VisDepthPSCBStruct cbVisDepth;
		cbVisDepth.mView = XMMatrixTranspose( XMLoadFloat4x4( &_ViewMat ));
		cbVisDepth.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
		cbVisDepth.ProjectionParams.x = _Far/(_Far - _Near);
		cbVisDepth.ProjectionParams.y = _Near/(_Near - _Far);

		_ImmediateContext->UpdateSubresource( _VisDpethPSCB, 0, NULL, &cbVisDepth, 0, 0 );
		_ImmediateContext->PSSetConstantBuffers( 0, 1, &_VisDpethPSCB );

		DrawFullScreenQuad11(_VisDpethPS, _Width/4, _Height/4, 0, _Height*0.75);
	}
	
	_SwapChain->Present( 0, 0 );
}

void Engine::StartRenderingFrameBuffer(bool bClearColor, bool bClearDepth, bool bReadOnlyDepth)
{
	ID3D11RenderTargetView* aRTViewsCombine[ 1] = { _FrameBufferTexture->GetRTV() };
	if(bReadOnlyDepth)
		_ImmediateContext->OMSetRenderTargets( 1, aRTViewsCombine, _DepthTexture->GetReadOnlyDepthStencilView() );
	else
	_ImmediateContext->OMSetRenderTargets( 1, aRTViewsCombine, _DepthTexture->GetDepthStencilView() );

	if(bClearColor)
	{
		float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
		_ImmediateContext->ClearRenderTargetView( _FrameBufferTexture->GetRTV(), ClearColor );
	}
	if(bClearDepth)
		_ImmediateContext->ClearDepthStencilView( _DepthTexture->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
}

void Engine::StartRenderingGBuffers()
{
	ID3D11ShaderResourceView* aSRS[2] = {NULL, NULL};
	_ImmediateContext->PSSetShaderResources( 0, 2, aSRS );

	ID3D11RenderTargetView* aRTViews[ 2 ] = { _SceneColorTexture->GetRTV(), _WorldNormalTexture->GetRTV() };
	_ImmediateContext->OMSetRenderTargets( 2, aRTViews, _DepthTexture->GetDepthStencilView() );     
	SET_DEPTHSTENCIL_STATE(DS_GBUFFER_PASS);
	SET_BLEND_STATE(BS_NORMAL);

	// Just clear the backbuffer
	float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
	_ImmediateContext->ClearRenderTargetView( _SceneColorTexture->GetRTV(), ClearColor );
	float ClearNormalColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
	_ImmediateContext->ClearRenderTargetView( _WorldNormalTexture->GetRTV() , ClearNormalColor );
	_ImmediateContext->ClearDepthStencilView( _DepthTexture->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0 );
}

void Engine::StartRenderingLightingBuffer(bool bClear)
{
	ID3D11ShaderResourceView* aSRSLit[2] = {NULL, NULL};
	_ImmediateContext->VSSetShaderResources( 0, 2, aSRSLit );

	ID3D11RenderTargetView* aRTViewsLit[ 1] = { _LitTexture->GetRTV() };
	_ImmediateContext->OMSetRenderTargets( 1, aRTViewsLit, _DepthTexture->GetReadOnlyDepthStencilView() );     

	if(bClear)
	{
		float LitClearColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
		_ImmediateContext->ClearRenderTargetView( _LitTexture->GetRTV() , LitClearColor );
	}
}

struct Triangle 
{
    XMVECTOR pt[3];
    BOOL culled;
};

void CreateFrustumPointsFromCascadeInterval( float fCascadeIntervalBegin, 
	FLOAT fCascadeIntervalEnd, 
	XMMATRIX &vProjection,
	XMVECTOR* pvCornerPointsWorld ) 
{

	XNA::Frustum vViewFrust;
	ComputeFrustumFromProjection( &vViewFrust, &vProjection );
	vViewFrust.Near = -fCascadeIntervalBegin;
	vViewFrust.Far = -fCascadeIntervalEnd;

	static const XMVECTORU32 vGrabY = {0x00000000,0xFFFFFFFF,0x00000000,0x00000000};
	static const XMVECTORU32 vGrabX = {0xFFFFFFFF,0x00000000,0x00000000,0x00000000};

	XMVECTORF32 vRightTop = {vViewFrust.RightSlope,vViewFrust.TopSlope,1.0f,1.0f};
	XMVECTORF32 vLeftBottom = {vViewFrust.LeftSlope,vViewFrust.BottomSlope,1.0f,1.0f};
	XMVECTORF32 vNear = {vViewFrust.Near,vViewFrust.Near,vViewFrust.Near,1.0f};
	XMVECTORF32 vFar = {vViewFrust.Far,vViewFrust.Far,vViewFrust.Far,1.0f};
	XMVECTOR vRightTopNear = XMVectorMultiply( vRightTop, vNear );
	XMVECTOR vRightTopFar = XMVectorMultiply( vRightTop, vFar );
	XMVECTOR vLeftBottomNear = XMVectorMultiply( vLeftBottom, vNear );
	XMVECTOR vLeftBottomFar = XMVectorMultiply( vLeftBottom, vFar );

	pvCornerPointsWorld[0] = vRightTopNear;
	pvCornerPointsWorld[1] = XMVectorSelect( vRightTopNear, vLeftBottomNear, vGrabX );
	pvCornerPointsWorld[2] = vLeftBottomNear;
	pvCornerPointsWorld[3] = XMVectorSelect( vRightTopNear, vLeftBottomNear,vGrabY );

	pvCornerPointsWorld[4] = vRightTopFar;
	pvCornerPointsWorld[5] = XMVectorSelect( vRightTopFar, vLeftBottomFar, vGrabX );
	pvCornerPointsWorld[6] = vLeftBottomFar;
	pvCornerPointsWorld[7] = XMVectorSelect( vRightTopFar ,vLeftBottomFar, vGrabY );

}

void ComputeNearAndFar( FLOAT& fNearPlane, 
                                        FLOAT& fFarPlane, 
                                        FXMVECTOR vLightCameraOrthographicMin, 
                                        FXMVECTOR vLightCameraOrthographicMax, 
                                        XMVECTOR* pvPointsInCameraView ) 
{

    // Initialize the near and far planes
    fNearPlane = FLT_MAX;
    fFarPlane = -FLT_MAX;
    
    Triangle triangleList[16];
    INT iTriangleCnt = 1;

    triangleList[0].pt[0] = pvPointsInCameraView[0];
    triangleList[0].pt[1] = pvPointsInCameraView[1];
    triangleList[0].pt[2] = pvPointsInCameraView[2];
    triangleList[0].culled = false;

    // These are the indices used to tesselate an AABB into a list of triangles.
    static const INT iAABBTriIndexes[] = 
    {
        0,1,2,  1,2,3,
        4,5,6,  5,6,7,
        0,2,4,  2,4,6,
        1,3,5,  3,5,7,
        0,1,4,  1,4,5,
        2,3,6,  3,6,7 
    };

    INT iPointPassesCollision[3];

    // At a high level: 
    // 1. Iterate over all 12 triangles of the AABB.  
    // 2. Clip the triangles against each plane. Create new triangles as needed.
    // 3. Find the min and max z values as the near and far plane.
    
    //This is easier because the triangles are in camera spacing making the collisions tests simple comparisions.
    
    float fLightCameraOrthographicMinX = XMVectorGetX( vLightCameraOrthographicMin );
    float fLightCameraOrthographicMaxX = XMVectorGetX( vLightCameraOrthographicMax ); 
    float fLightCameraOrthographicMinY = XMVectorGetY( vLightCameraOrthographicMin );
    float fLightCameraOrthographicMaxY = XMVectorGetY( vLightCameraOrthographicMax );
    
    for( INT AABBTriIter = 0; AABBTriIter < 12; ++AABBTriIter ) 
    {

        triangleList[0].pt[0] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 0 ] ];
        triangleList[0].pt[1] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 1 ] ];
        triangleList[0].pt[2] = pvPointsInCameraView[ iAABBTriIndexes[ AABBTriIter*3 + 2 ] ];
        iTriangleCnt = 1;
        triangleList[0].culled = FALSE;

        // Clip each invidual triangle against the 4 frustums.  When ever a triangle is clipped into new triangles, 
        //add them to the list.
        for( INT frustumPlaneIter = 0; frustumPlaneIter < 4; ++frustumPlaneIter ) 
        {

            FLOAT fEdge;
            INT iComponent;
            
            if( frustumPlaneIter == 0 ) 
            {
                fEdge = fLightCameraOrthographicMinX; // todo make float temp
                iComponent = 0;
            } 
            else if( frustumPlaneIter == 1 ) 
            {
                fEdge = fLightCameraOrthographicMaxX;
                iComponent = 0;
            } 
            else if( frustumPlaneIter == 2 ) 
            {
                fEdge = fLightCameraOrthographicMinY;
                iComponent = 1;
            } 
            else 
            {
                fEdge = fLightCameraOrthographicMaxY;
                iComponent = 1;
            }

            for( INT triIter=0; triIter < iTriangleCnt; ++triIter ) 
            {
                // We don't delete triangles, so we skip those that have been culled.
                if( !triangleList[triIter].culled ) 
                {
                    INT iInsideVertCount = 0;
                    XMVECTOR tempOrder;
                    // Test against the correct frustum plane.
                    // This could be written more compactly, but it would be harder to understand.
                    
                    if( frustumPlaneIter == 0 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetX( triangleList[triIter].pt[triPtIter] ) >
                                XMVectorGetX( vLightCameraOrthographicMin ) ) 
                            { 
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if( frustumPlaneIter == 1 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetX( triangleList[triIter].pt[triPtIter] ) < 
                                XMVectorGetX( vLightCameraOrthographicMax ) )
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else
                            { 
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else if( frustumPlaneIter == 2 ) 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetY( triangleList[triIter].pt[triPtIter] ) > 
                                XMVectorGetY( vLightCameraOrthographicMin ) ) 
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }
                    else 
                    {
                        for( INT triPtIter=0; triPtIter < 3; ++triPtIter ) 
                        {
                            if( XMVectorGetY( triangleList[triIter].pt[triPtIter] ) < 
                                XMVectorGetY( vLightCameraOrthographicMax ) ) 
                            {
                                iPointPassesCollision[triPtIter] = 1;
                            }
                            else 
                            {
                                iPointPassesCollision[triPtIter] = 0;
                            }
                            iInsideVertCount += iPointPassesCollision[triPtIter];
                        }
                    }

                    // Move the points that pass the frustum test to the begining of the array.
                    if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[0];   
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;            
                        iPointPassesCollision[1] = FALSE;            
                    }
                    if( iPointPassesCollision[2] && !iPointPassesCollision[1] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[1];   
                        triangleList[triIter].pt[1] = triangleList[triIter].pt[2];
                        triangleList[triIter].pt[2] = tempOrder;
                        iPointPassesCollision[1] = TRUE;            
                        iPointPassesCollision[2] = FALSE;                        
                    }
                    if( iPointPassesCollision[1] && !iPointPassesCollision[0] ) 
                    {
                        tempOrder =  triangleList[triIter].pt[0];   
                        triangleList[triIter].pt[0] = triangleList[triIter].pt[1];
                        triangleList[triIter].pt[1] = tempOrder;
                        iPointPassesCollision[0] = TRUE;            
                        iPointPassesCollision[1] = FALSE;            
                    }
                    
                    if( iInsideVertCount == 0 ) 
                    { // All points failed. We're done,  
                        triangleList[triIter].culled = true;
                    }
                    else if( iInsideVertCount == 1 ) 
                    {// One point passed. Clip the triangle against the Frustum plane
                        triangleList[triIter].culled = false;
                        
                        // 
                        XMVECTOR vVert0ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[0];
                        XMVECTOR vVert0ToVert2 = triangleList[triIter].pt[2] - triangleList[triIter].pt[0];
                        
                        // Find the collision ratio.
                        FLOAT fHitPointTimeRatio = fEdge - XMVectorGetByIndex( triangleList[triIter].pt[0], iComponent ) ;
                        // Calculate the distance along the vector as ratio of the hit ratio to the component.
                        FLOAT fDistanceAlongVector01 = fHitPointTimeRatio / XMVectorGetByIndex( vVert0ToVert1, iComponent );
                        FLOAT fDistanceAlongVector02 = fHitPointTimeRatio / XMVectorGetByIndex( vVert0ToVert2, iComponent );
                        // Add the point plus a percentage of the vector.
                        vVert0ToVert1 *= fDistanceAlongVector01;
                        vVert0ToVert1 += triangleList[triIter].pt[0];
                        vVert0ToVert2 *= fDistanceAlongVector02;
                        vVert0ToVert2 += triangleList[triIter].pt[0];

                        triangleList[triIter].pt[1] = vVert0ToVert2;
                        triangleList[triIter].pt[2] = vVert0ToVert1;

                    }
                    else if( iInsideVertCount == 2 ) 
                    { // 2 in  // tesselate into 2 triangles
                        

                        // Copy the triangle\(if it exists) after the current triangle out of
                        // the way so we can override it with the new triangle we're inserting.
                        triangleList[iTriangleCnt] = triangleList[triIter+1];

                        triangleList[triIter].culled = false;
                        triangleList[triIter+1].culled = false;
                        
                        // Get the vector from the outside point into the 2 inside points.
                        XMVECTOR vVert2ToVert0 = triangleList[triIter].pt[0] - triangleList[triIter].pt[2];
                        XMVECTOR vVert2ToVert1 = triangleList[triIter].pt[1] - triangleList[triIter].pt[2];
                        
                        // Get the hit point ratio.
                        FLOAT fHitPointTime_2_0 =  fEdge - XMVectorGetByIndex( triangleList[triIter].pt[2], iComponent );
                        FLOAT fDistanceAlongVector_2_0 = fHitPointTime_2_0 / XMVectorGetByIndex( vVert2ToVert0, iComponent );
                        // Calcaulte the new vert by adding the percentage of the vector plus point 2.
                        vVert2ToVert0 *= fDistanceAlongVector_2_0;
                        vVert2ToVert0 += triangleList[triIter].pt[2];
                        
                        // Add a new triangle.
                        triangleList[triIter+1].pt[0] = triangleList[triIter].pt[0];
                        triangleList[triIter+1].pt[1] = triangleList[triIter].pt[1];
                        triangleList[triIter+1].pt[2] = vVert2ToVert0;
                        
                        //Get the hit point ratio.
                        FLOAT fHitPointTime_2_1 =  fEdge - XMVectorGetByIndex( triangleList[triIter].pt[2], iComponent ) ;
                        FLOAT fDistanceAlongVector_2_1 = fHitPointTime_2_1 / XMVectorGetByIndex( vVert2ToVert1, iComponent );
                        vVert2ToVert1 *= fDistanceAlongVector_2_1;
                        vVert2ToVert1 += triangleList[triIter].pt[2];
                        triangleList[triIter].pt[0] = triangleList[triIter+1].pt[1];
                        triangleList[triIter].pt[1] = triangleList[triIter+1].pt[2];
                        triangleList[triIter].pt[2] = vVert2ToVert1;
                        // Cncrement triangle count and skip the triangle we just inserted.
                        ++iTriangleCnt;
                        ++triIter;

                    
                    }
                    else 
                    { // all in
                        triangleList[triIter].culled = false;

                    }
                }// end if !culled loop            
            }
        }
        for( INT index=0; index < iTriangleCnt; ++index ) 
        {
            if( !triangleList[index].culled ) 
            {
                // Set the near and far plan and the min and max z values respectivly.
                for( int vertind = 0; vertind < 3; ++ vertind ) 
                {
                    float fTriangleCoordZ = XMVectorGetZ( triangleList[index].pt[vertind] );
                    if( fNearPlane > fTriangleCoordZ ) 
                    {
                        fNearPlane = fTriangleCoordZ;
                    }
                    if( fFarPlane  <fTriangleCoordZ ) 
                    {
                        fFarPlane = fTriangleCoordZ;
                    }
                }
            }
        }
    }    

}

void CreateAABBPoints( XMVECTOR* vAABBPoints, FXMVECTOR vCenter, FXMVECTOR vExtents )
{
    //This map enables us to use a for loop and do vector math.
    static const XMVECTORF32 vExtentsMap[] = 
    { 
        {1.0f, 1.0f, -1.0f, 1.0f}, 
        {-1.0f, 1.0f, -1.0f, 1.0f}, 
        {1.0f, -1.0f, -1.0f, 1.0f}, 
        {-1.0f, -1.0f, -1.0f, 1.0f}, 
        {1.0f, 1.0f, 1.0f, 1.0f}, 
        {-1.0f, 1.0f, 1.0f, 1.0f}, 
        {1.0f, -1.0f, 1.0f, 1.0f}, 
        {-1.0f, -1.0f, 1.0f, 1.0f} 
    };
    
    for( INT index = 0; index < 8; ++index ) 
    {
        vAABBPoints[index] = XMVectorMultiplyAdd(vExtentsMap[index], vExtents, vCenter ); 
    }
}


void Engine::RenderShadowMap()
{
	ID3D11ShaderResourceView* aSRS[3] = {NULL, NULL, NULL};
	_ImmediateContext->PSSetShaderResources( 0, 3, aSRS );

	ID3D11RenderTargetView* aRTV[ 1] = { NULL };
	_ImmediateContext->OMSetRenderTargets( 1, aRTV, _ShadowDepthTexture->GetDepthStencilView() );
	_ImmediateContext->ClearDepthStencilView( _ShadowDepthTexture->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	SET_DEPTHSTENCIL_STATE(DS_GBUFFER_PASS);

	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
	UINT nViewPorts = 1;
	_ImmediateContext->RSGetViewports( &nViewPorts, vpOld );

	// Setup the viewport to match the backbuffer
	D3D11_VIEWPORT vp;
	vp.Width = (float)_ShadowMapSize;
	vp.Height = (float)_ShadowMapSize;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_ImmediateContext->RSSetViewports( 1, &vp );
    // Initialize the view matrix

	_SunShadowNear = 10.f;
	_SunShadowFar = 2000.f;

	//XMVECTOR Min = XMLoadFloat3(&_StaticMeshComponent->_AABBMin);
	//XMVECTOR Max = XMLoadFloat3(&_StaticMeshComponent->_AABBMax);
	//XMVECTOR Center = (Min + Max)*0.5f;
	//XMVECTOR Extents = (Max - Min) * 0.5f;

	XMVECTOR Det;
	XMMATRIX ViewMatInv = XMMatrixInverse(&Det, XMLoadFloat4x4(&_ViewMat));

	XMVECTOR Eye =  XMLoadFloat3(&XMFLOAT3(0.f, 0.f, 0.f));
	XMVECTOR LightDir = XMLoadFloat3(&_SunLight->_LightDirection);
	XMVECTOR Up = XMLoadFloat3(&XMFLOAT3(0.f, 1.f, 0.f));
	
	XMVECTOR Center = XMVectorSet(0.f, 0.f, 0.f, 0.f);;
	XMVECTOR vFrustumPoints[8];

	float fFrustumIntervalBegin, fFrustumIntervalEnd;

	fFrustumIntervalBegin = 10;
	fFrustumIntervalEnd = 650;
	CreateFrustumPointsFromCascadeInterval( fFrustumIntervalBegin, fFrustumIntervalEnd, XMLoadFloat4x4(&_ProjectionMat), vFrustumPoints); 
	for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
	{
		// world space
        vFrustumPoints[icpIndex] = XMVector4Transform ( vFrustumPoints[icpIndex], ViewMatInv );
		Center += vFrustumPoints[icpIndex];
	}

	Center /= 8;

//	XMMATRIX LightRotation = XMMatrixLookAtRH(XMLoadFloat3(&XMFLOAT3(0.f, 0.f, 0.f)), LightDir, Up); 
	//XMMATRIX LightRotationInv = XMMatrixInverse(&Det, LightRotation);
	XMMATRIX LightView = XMMatrixLookAtRH( Center - LightDir, Center, Up );
//	XMMATRIX LightViewInv = XMMatrixInverse(&Det, LightView);




	XMVECTOR vLightCameraOrthographicMin;  // light space frustrum aabb 
	XMVECTOR vLightCameraOrthographicMax;
	vLightCameraOrthographicMin = XMLoadFloat3(&XMFLOAT3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX));;
	vLightCameraOrthographicMax = XMLoadFloat3(&XMFLOAT3(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX));;
	XMVECTOR vTempTranslatedCornerPoint;

	for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
	{
		// world space
        //vFrustumPoints[icpIndex] = XMVector4Transform ( vFrustumPoints[icpIndex], ViewMatInv );
		// light space
		vTempTranslatedCornerPoint = XMVector4Transform ( vFrustumPoints[icpIndex], LightView );
		vLightCameraOrthographicMin = XMVectorMin ( vTempTranslatedCornerPoint, vLightCameraOrthographicMin );
		vLightCameraOrthographicMax = XMVectorMax ( vTempTranslatedCornerPoint, vLightCameraOrthographicMax );
	}

	XMVECTOR BoxSize = (vLightCameraOrthographicMax - vLightCameraOrthographicMin);

	/*XMVECTOR LightPosition = (vLightCameraOrthographicMin + vLightCameraOrthographicMax)/2;
	XMVectorSetZ(LightPosition, XMVectorGetX(vLightCameraOrthographicMin));
	XMVector4Transform(LightPosition, LightRotationInv);*/

	//XMMATRIX LightView = XMMatrixLookAtRH( LightPosition, LightPosition + LightDir, Up );
	
	float ShadowNear = 0.0f;
    float ShadowFar = 10000.0f;


	XMMATRIX LightProjection = XMMatrixOrthographicOffCenterRH( 
		XMVectorGetX( vLightCameraOrthographicMin )
		,  XMVectorGetX( vLightCameraOrthographicMax )
		, XMVectorGetY(vLightCameraOrthographicMin)
		, XMVectorGetY(vLightCameraOrthographicMax)
		, -XMVectorGetZ( vLightCameraOrthographicMax )
		, -XMVectorGetZ( vLightCameraOrthographicMin )
		);

	/*XMMATRIX LightProjection = XMMatrixOrthographicOffCenterRH( 
		-XMVectorGetX( BoxSize )
		,XMVectorGetX( BoxSize )
		,-XMVectorGetY(BoxSize)
		,XMVectorGetY(BoxSize)
		,-XMVectorGetZ( BoxSize )
		,XMVectorGetZ( BoxSize ));*/
	//XMMATRIX Projection = XMMatrixPerspectiveFovRH( XM_PIDIV2, _ShadowMapSize /_ShadowMapSize, _SunShadowNear, _SunShadowFar );

	XMStoreFloat4x4(&_SunShadowMat, LightView);
	XMStoreFloat4x4(&_SunShadowProjectionMat, LightProjection);

	for(unsigned int i=0;i<_StaticMeshArray.size();i++)
	{
		_GBufferDrawer->DrawStaticMesh(_StaticMeshArray[i], _SunShadowMat, _SunShadowProjectionMat);
	}

	if(_GSkeletalMeshComponent)
	{
		for(unsigned int i=0;i<_GSkeletalMeshComponent->_RenderDataArray.size();i++)
		{
			_GBufferDrawer->DrawSkeletalMeshData(_GSkeletalMeshComponent->_RenderDataArray[i], _SunShadowMat, _SunShadowProjectionMat);
		}
	}

	_ImmediateContext->RSSetViewports( nViewPorts, vpOld );
}

void Engine::RenderDeferredShadow()
{
	ID3D11RenderTargetView* aRTViewsLit[ 1] = { _DeferredShadowTexture->GetRTV() };
	_ImmediateContext->OMSetRenderTargets( 1, aRTViewsLit, NULL);     

	//float ShadowClearColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
	float ShadowClearColor[4] = { 1.f, 1.f, 1.f, 1.f }; //red,green,blue,alpha
	_ImmediateContext->ClearRenderTargetView( _DeferredShadowTexture->GetRTV() , ShadowClearColor );

	ID3D11ShaderResourceView* aSRVVis[2] = {_DepthTexture->GetSRV(), _ShadowDepthTexture->GetSRV()};
	_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis );

	SET_DEPTHSTENCIL_STATE(DS_LIGHTING_PASS);
	SET_BLEND_STATE(BS_SHADOW); // litbuffer.a -= shadow mask
	SET_PS_SAMPLER(0, SS_LINEAR);
	SET_PS_SAMPLER(1, SS_SHADOW);
	//SET_BLEND_STATE(BS_LIGHTING); 

	
	DeferredShadowPSCBStruct cbShadow;

	cbShadow.Projection = XMMatrixTranspose( XMLoadFloat4x4(&_ProjectionMat));
	cbShadow.ProjectionParams.x = _Far/(_Far - _Near);
	cbShadow.ProjectionParams.y = _Near/(_Near - _Far);
	cbShadow.ProjectionParams.z = _Far;
	cbShadow.ViewportParams.x = (float)_Width;
	cbShadow.ViewportParams.y = (float)_Height;
	cbShadow.ViewportParams.z = (float)_ShadowMapSize;

	XMVECTOR Det;
	XMMATRIX InvViewMatrix = XMMatrixInverse(&Det, XMLoadFloat4x4(&_ViewMat));
	cbShadow.ShadowMatrix = XMMatrixTranspose(InvViewMatrix * XMLoadFloat4x4(&_SunShadowMat) * XMLoadFloat4x4(&_SunShadowProjectionMat));

	_ImmediateContext->UpdateSubresource( _DeferredShadowPSCB, 0, NULL, &cbShadow, 0, 0 );
	_ImmediateContext->PSSetConstantBuffers( 0, 1, &_DeferredShadowPSCB );
	DrawFullScreenQuad11(_DeferredShadowPS, _Width, _Height);

	SET_PS_SAMPLER(0, SS_LINEAR);
}

float Engine::_GetTimeSeconds()
{
	LARGE_INTEGER CurrentTime;
	QueryPerformanceCounter(&CurrentTime);
	return (float)(CurrentTime.QuadPart)/(float)_Freq.QuadPart;
}

