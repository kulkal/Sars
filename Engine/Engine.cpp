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
#include "FpsCamera.h"
#include "Input.h"
#include "DeferredShadowPixelShader.h"
#include "DeferredPointLightPixelShader.h"
#include "DeferredDirLightPixelShader.h"

struct SCREEN_VERTEX
{
	XMFLOAT4 pos;
	XMFLOAT2 tex;
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
	,_DeferredShadowTexture(NULL)
	,_StaticMeshComponent(NULL)
	,_CurrentCamera(NULL)
	,_Input(NULL)
	,_DeferredDirPS(NULL)
	,_DeferredPointPS(NULL)
	,_DeferredShadowPS(NULL)
	
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	QueryPerformanceFrequency(&_Freq);
	QueryPerformanceCounter(&_PrevTime);
	//_Near = 10.f;
	//_Far = 2000.f;
	//_ShadowMapSize = 1024;
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
	
	if(_DeferredShadowPS) delete _DeferredShadowPS;
	if(_DeferredPointPS) delete _DeferredPointPS;
	if(_DeferredDirPS) delete _DeferredDirPS;


	if(_SimpleDrawer) delete _SimpleDrawer;
	if(_GBufferDrawer) delete _GBufferDrawer;

	if(_LineBatcher) delete _LineBatcher;

	if(_WorldNormalTexture) delete _WorldNormalTexture;
	if(_DepthTexture) delete _DepthTexture;
	if(_FrameBufferTexture) delete _FrameBufferTexture;
	if(_SceneColorTexture) delete _SceneColorTexture;
	if(_LitTexture) delete _LitTexture;
//	if(_ShadowDepthTexture) delete _ShadowDepthTexture;
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

	if(_CurrentCamera) delete _CurrentCamera;
	
	if(_Input)
	{
		_Input->Release();
		delete _Input;
	}

	for(unsigned int i=0;i<_CascadeArray.size();i++)
	{
		ShadowCascadeInfo* ShadowInfo = _CascadeArray[i];
		delete ShadowInfo;
	}
}

void Engine::InitDevice()
{
	HRESULT hr;
	RECT rc;
	GetClientRect( _hWnd, &rc );
	_Width = (float)(rc.right - rc.left);
	_Height = (float)(rc.bottom - rc.top);

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
	sd.BufferDesc.Width = (UINT)_Width;
	sd.BufferDesc.Height = (UINT)_Height;
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

	UINT FrameBufferWidth = (UINT)_Width;
	UINT FrameBufferHeight = (UINT)_Height;
	// scene color
	CD3D11_TEXTURE2D_DESC DescSceneColorTex(DXGI_FORMAT_R16G16B16A16_FLOAT, FrameBufferWidth, FrameBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescSceneColorSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescSceneColorTex.Format, 0,  DescSceneColorTex.MipLevels);
	_SceneColorTexture = new Texture2D(DescSceneColorTex, DescSceneColorSRV, true);

	// lit
	CD3D11_TEXTURE2D_DESC DescLitTex(DXGI_FORMAT_R16G16B16A16_FLOAT, FrameBufferWidth, FrameBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescLitSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescSceneColorTex.Format, 0, DescLitTex.MipLevels);
	_LitTexture = new Texture2D(DescLitTex, DescLitSRV, true);

	// world normal
	CD3D11_TEXTURE2D_DESC DescWordNormalTex(DXGI_FORMAT_R16G16B16A16_FLOAT, FrameBufferWidth, FrameBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescWorldNormalSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DescWordNormalTex.Format);
	_WorldNormalTexture = new Texture2D(DescWordNormalTex, DescWorldNormalSRV, true);

	// depth stencil texture
	CD3D11_TEXTURE2D_DESC DescDepthTex(DXGI_FORMAT_R24G8_TYPELESS, FrameBufferWidth, FrameBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_DEPTH_STENCIL_VIEW_DESC  DescDSV(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT, 0, 0, 0,0) ;
	CD3D11_SHADER_RESOURCE_VIEW_DESC DescDepthSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	_DepthTexture = new TextureDepth2D(DescDepthTex, DescDSV, DescDepthSRV);

	// shadow result
	CD3D11_TEXTURE2D_DESC DescShadowTex(DXGI_FORMAT_R8G8B8A8_UNORM, FrameBufferWidth, FrameBufferHeight, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
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

	
	_CombineLitPS = CreatePixelShaderSimple("ComblineShader.fx");

	_DeferredDirPS  = new DeferredDirLightPixelShader("DeferredDirectional.fx", "PS");
	_DeferredPointPS = new DeferredPointLightPixelShader("DeferredPoint.fx", "PS");
	_DeferredShadowPS = new DeferredShadowPixelShader("DeferredShadow.fx", "PS");

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

	FbxFileImporter FbxImporterObj2("sponza\\sponza.fbx");
	//FbxFileImporter FbxImporterObj2("other.fbx");
	FbxImporterObj2.ImportStaticMesh(_StaticMeshArray);

	_StaticMeshComponent = new StaticMeshComponent;
	for(unsigned int i=0;i<_StaticMeshArray.size();i++)
	{
		_StaticMeshComponent->AddStaticMesh(_StaticMeshArray[i]);
	}

	//cout_debug("staticmesh aabb min: %f %f %f\n", _StaticMeshComponent->_AABBMin.x, _StaticMeshComponent->_AABBMin.y, _StaticMeshComponent->_AABBMin.z);
	//cout_debug("staticmesh aabb max: %f %f %f\n", _StaticMeshComponent->_AABBMax.x, _StaticMeshComponent->_AABBMax.y, _StaticMeshComponent->_AABBMax.z);

	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile( GEngine->_Device, L"seafloor.dds", NULL, NULL, &_TextureRV, NULL );
	if( FAILED( hr ) )
		assert(false);

	GStateManager = new StateManager;
	GStateManager->Init();

	_SunLight = new DirectionalLightComponent(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.F), XMFLOAT3(-1.0f, -4.f, -1.f) );
	_LightCompArray.push_back(_SunLight);

	PointLightComponent* PointLight1 = new PointLightComponent(XMFLOAT4( 0.f, 1.f, 0.f, 1.0f), XMFLOAT3(  0.f, 0, 100.f  ), 200.f);
	_LightCompArray.push_back(PointLight1);

	PointLightComponent* PointLight2 = new PointLightComponent(XMFLOAT4(  0.f, 0.f, 1.f, 1.0f), XMFLOAT3( 100.f, 50.f, 0.f ), 200.f);
	_LightCompArray.push_back(PointLight2);

	_Input = new Input;
	_Input->Create(_hWnd, (long)_Width, (long)_Height, 0, 0);

	_CurrentCamera = new FpsCamera(XMFLOAT3(0.f, 250.f, 250.f), 0.f, -XM_PI/4);

	_CascadeArray.resize(3);
	_CascadeArray[0] = new ShadowCascadeInfo(0, 80, 1024);
	_CascadeArray[1] = new ShadowCascadeInfo(80, 250, 1024);
	_CascadeArray[2] = new ShadowCascadeInfo(250, 800, 1024);
	//_CascadeArray[3] = new ShadowCascadeInfo(1050, 3250, 1024);

	//_CascadeArray[2] = new ShadowCascadeInfo(250, 2500, 256);

}

ID3D11PixelShader* Engine::CreatePixelShaderSimple( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
{
	ID3D11PixelShader* PS;
	HRESULT hr;
	int nLen = strlen(szFileName)+1;
	wchar_t WFileName[1024];
	size_t RetSize;
	mbstowcs_s(&RetSize, WFileName, 1024, szFileName, nLen);

	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(WFileName, pDefines,szFuncName, "ps_4_0", &pPSBlob );
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

void Engine::DrawFullScreenQuad11( ID3D11PixelShader* pPS, float Width, float Height, float TopLeftX, float TopLeftY)
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
	vp.TopLeftX = (float)TopLeftX;
	vp.TopLeftY = (float)TopLeftY;
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

	WCHAR Path[MAX_PATH];
	wsprintf(Path, L"Shaders\\%s", szFileName);

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile( Path, pDefines, NULL, szEntryPoint, szShaderModel, 
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
	if(_Input) _Input->Update();

	if(_CascadeArray.size() > 0)
	{
		if(_CascadeArray[1] != NULL && _Input->IsKeyDn(DIK_1) )
		{
			_CascadeArray[1]->_bEnabled = !_CascadeArray[1]->_bEnabled;
		}

		if(_CascadeArray[2] != NULL  &&_Input->IsKeyDn(DIK_2) )
		{
			_CascadeArray[2]->_bEnabled = !_CascadeArray[2]->_bEnabled;
		}

		if(_CascadeArray[0]  != NULL && _Input->IsKeyDn(DIK_0) )
		{
			_CascadeArray[0]->_bEnabled = !_CascadeArray[0]->_bEnabled;
		}
	}

	LARGE_INTEGER CurrentTime;

	QueryPerformanceCounter(&CurrentTime);
	_DeltaSeconds = (float)(CurrentTime.QuadPart - _PrevTime.QuadPart)/(float)_Freq.QuadPart;
	_TimeSeconds =	(float)(CurrentTime.QuadPart)/(float)_Freq.QuadPart;
	//cout_debug("delta seconds: %f\n", _DeltaSeconds);

	if(_GSkeletalMeshComponent) _GSkeletalMeshComponent->Tick(_DeltaSeconds);

	if(_CurrentCamera) 
	{
		_CurrentCamera->Tick(_DeltaSeconds);
	}

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

	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;

	if(_CurrentCamera) _CurrentCamera->CalcViewInfo(ViewMatrix, ProjectionMatrix, _Width, _Height);
	XMStoreFloat4x4(&_ViewMat, ViewMatrix);
	XMStoreFloat4x4(&_ProjectionMat, ProjectionMatrix);

	SET_RASTERIZER_STATE(RS_NORMAL);
	// draw scene into g-buffer
	for(unsigned int i=0;i<_StaticMeshComponent->_StaticMeshArray.size();i++)
	{
		StaticMesh* Mesh = _StaticMeshComponent->_StaticMeshArray[i];
		_GBufferDrawer->DrawStaticMesh(Mesh, ViewMatrix, ProjectionMatrix);
	}

	if(_GSkeletalMeshComponent)
	{
		for(unsigned int i=0;i<_GSkeletalMeshComponent->_RenderDataArray.size();i++)
		{
			_GBufferDrawer->DrawSkeletalMeshData(_GSkeletalMeshComponent->_RenderDataArray[i], ViewMatrix, ProjectionMatrix);
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
		Light->RenderLightDeferred(_CurrentCamera);
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
		float Near = _CurrentCamera->GetNear();
		float Far = _CurrentCamera->GetFar();
		VisDepthPSCBStruct cbVisDepth;
		cbVisDepth.mView = XMMatrixTranspose( XMLoadFloat4x4( &_ViewMat ));
		cbVisDepth.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
		cbVisDepth.ProjectionParams.x = Far/(Far - Near);
		cbVisDepth.ProjectionParams.y = Near/(Near - Far);
		_ImmediateContext->UpdateSubresource( _VisDpethPSCB, 0, NULL, &cbVisDepth, 0, 0 );
		_ImmediateContext->PSSetConstantBuffers( 0, 1, &_VisDpethPSCB );

		DrawFullScreenQuad11(_VisDpethPS, _Width/4, _Height/4, _Width*0.75f, 0);
	}

	_VisualizeWorldNormal = true;
	if(_VisualizeWorldNormal)
	{
		DrawFullScreenQuad11(_VisNormalPS, _Width/4, _Height/4);
	}

	bool _VisualizeShadow = true;
	if(_VisualizeShadow)
	{
		ID3D11ShaderResourceView* aSRVVis[1] = {_DeferredShadowTexture->GetSRV(), };
		_ImmediateContext->PSSetShaderResources( 0, 1, aSRVVis );
		DrawFullScreenQuad11(_VisNormalPS, _Width/4, _Height/4, _Width*0.75f, _Height*0.75f);
	}

	bool _VisualizeShadowMap = true;
	if(_VisualizeShadowMap)
	{
		float Near = _CurrentCamera->GetNear();
		float Far = _CurrentCamera->GetFar();
		

		VisDepthPSCBStruct cbVisDepth;
		cbVisDepth.mView = XMMatrixTranspose( XMLoadFloat4x4( &_ViewMat ));
		cbVisDepth.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
		cbVisDepth.ProjectionParams.x = Far/(Far - Near);
		cbVisDepth.ProjectionParams.y = Near/(Near - Far);

		_ImmediateContext->UpdateSubresource( _VisDpethPSCB, 0, NULL, &cbVisDepth, 0, 0 );
		_ImmediateContext->PSSetConstantBuffers( 0, 1, &_VisDpethPSCB );

		ID3D11ShaderResourceView* aSRVVis[2] = {NULL, _CascadeArray[0]->_ShadowDepthTexture->GetSRV()};
		_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis );
		DrawFullScreenQuad11(_VisDpethPS, _Width/4, _Height/4, 0.f, _Height*0.75f);

		ID3D11ShaderResourceView* aSRVVis2[2] = {NULL, _CascadeArray[1]->_ShadowDepthTexture->GetSRV()};
		_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis2 );
		DrawFullScreenQuad11(_VisDpethPS, _Width/4, _Height/4, 0.f, _Height*0.5f);

		ID3D11ShaderResourceView* aSRVVis3[2] = {NULL, _CascadeArray[2]->_ShadowDepthTexture->GetSRV()};
		_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis3 );
		DrawFullScreenQuad11(_VisDpethPS, _Width/4, _Height/4, _Width*0.25f, _Height*0.75f);
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
	XMVECTOR Det;
	XMMATRIX ViewMatInv = XMMatrixInverse(&Det, XMLoadFloat4x4(&_ViewMat));

	XMVECTOR LightDir = XMLoadFloat3(&_SunLight->_LightDirection);
	XMVECTOR Up = XMVectorSet(ViewMatInv._31, ViewMatInv._32, ViewMatInv._33, 1.f);//XMLoadFloat3(&XMFLOAT3(0.f, 1.f, 0.f));
	XMVECTOR Center = XMVectorSet(0.f, 0.f, 0.f, 0.f);

	for(unsigned int i=0;i<_CascadeArray.size();i++)
	{
		ShadowCascadeInfo* ShadowInfo = _CascadeArray[i];

		ID3D11ShaderResourceView* aSRS[3] = {NULL, NULL, NULL};
		_ImmediateContext->PSSetShaderResources( 0, 3, aSRS );

		ID3D11RenderTargetView* aRTV[ 1] = { NULL };
		_ImmediateContext->OMSetRenderTargets( 1, aRTV, ShadowInfo->_ShadowDepthTexture->GetDepthStencilView() );
		_ImmediateContext->ClearDepthStencilView( ShadowInfo->_ShadowDepthTexture->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		SET_DEPTHSTENCIL_STATE(DS_GBUFFER_PASS);

		D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
		UINT nViewPorts = 1;
		_ImmediateContext->RSGetViewports( &nViewPorts, vpOld );

		// Setup the viewport to match the backbuffer
		D3D11_VIEWPORT vp;
		vp.Width = (float)ShadowInfo->_TextureSize;
		vp.Height = (float)ShadowInfo->_TextureSize;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		_ImmediateContext->RSSetViewports( 1, &vp );
		
		XMVECTOR vFrustumPoints[8];
		float fFrustumIntervalBegin, fFrustumIntervalEnd;
		fFrustumIntervalBegin = ShadowInfo->_ViewNear;
		fFrustumIntervalEnd = ShadowInfo->_ViewFar;
		XMMATRIX ProjectionMat = XMLoadFloat4x4(&_ProjectionMat);
		CreateFrustumPointsFromCascadeInterval( fFrustumIntervalBegin, fFrustumIntervalEnd, ProjectionMat, vFrustumPoints); 

		XMMATRIX LightView = XMMatrixLookAtRH( Center - LightDir, Center, Up );

		XMVECTOR m_vSceneAABBMin = XMLoadFloat3(&_StaticMeshComponent->_AABBMin);
		XMVECTOR m_vSceneAABBMax = XMLoadFloat3(&_StaticMeshComponent->_AABBMax);

		XMVECTOR vSceneCenter = m_vSceneAABBMin + m_vSceneAABBMax;
		vSceneCenter *= 0.5f;
		XMVECTOR vSceneExtents = m_vSceneAABBMax - m_vSceneAABBMin;
		vSceneExtents *= 0.5f;    
		XMVECTOR vSceneAABBPointsLightSpace[8];

		// This function simply converts the center and extents of an AABB into 8 points
		CreateAABBPoints( vSceneAABBPointsLightSpace, vSceneCenter, vSceneExtents );
		// Transform the scene AABB to Light space.
		for( int index =0; index < 8; ++index ) 
		{
			vSceneAABBPointsLightSpace[index] = XMVector4Transform( vSceneAABBPointsLightSpace[index], LightView ); 
		}

		XMVECTOR vLightSpaceSceneAABBminValue = XMVectorSet(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);  // world space scene aabb 
		XMVECTOR vLightSpaceSceneAABBmaxValue = XMVectorSet(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);       
		// We calculate the min and max vectors of the scene in light space. The min and max "Z" values of the  
		// light space AABB can be used for the near and far plane. This is easier than intersecting the scene with the AABB
		// and in some cases provides similar results.
		for(int index=0; index< 8; ++index) 
		{
			vLightSpaceSceneAABBminValue = XMVectorMin( vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBminValue );
			vLightSpaceSceneAABBmaxValue = XMVectorMax( vSceneAABBPointsLightSpace[index], vLightSpaceSceneAABBmaxValue );
		}

		XMVECTOR vLightCameraOrthographicMin; 
		XMVECTOR vLightCameraOrthographicMax;
		vLightCameraOrthographicMin = XMVectorSet(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX, FLOAT_MAX);
		vLightCameraOrthographicMax = XMVectorSet(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX);

		XMVECTOR vTempTranslatedCornerPoint;

		for( int icpIndex=0; icpIndex < 8; ++icpIndex ) 
		{
			// world space
			vFrustumPoints[icpIndex] = XMVector4Transform ( vFrustumPoints[icpIndex], ViewMatInv );
			// light space
			vTempTranslatedCornerPoint = XMVector4Transform ( vFrustumPoints[icpIndex], LightView );
			vLightCameraOrthographicMin = XMVectorMin ( vTempTranslatedCornerPoint, vLightCameraOrthographicMin );
			vLightCameraOrthographicMax = XMVectorMax ( vTempTranslatedCornerPoint, vLightCameraOrthographicMax );
		}

		XMVECTOR vDiagonal = vFrustumPoints[0] - vFrustumPoints[6];
        vDiagonal = XMVector3Length( vDiagonal );
            
        // The bound is the length of the diagonal of the frustum interval.
        FLOAT fCascadeBound = XMVectorGetX( vDiagonal );
            
        // The offset calculated will pad the ortho projection so that it is always the same size 
        // and big enough to cover the entire cascade interval.
        XMVECTOR vBoarderOffset = ( vDiagonal - 
                                    ( vLightCameraOrthographicMax - vLightCameraOrthographicMin ) ) 
                                    * 0.5f;
        // Set the Z and W components to zero.
       vBoarderOffset *= XMVectorSet(1.f, 1.f, 0.f, 0.f);;
		
        // Add the offsets to the projection.
        vLightCameraOrthographicMax += vBoarderOffset;
        vLightCameraOrthographicMin -= vBoarderOffset;
		
		XMVECTOR vWorldUnitsPerTexel;
		FLOAT fWorldUnitsPerTexel = fCascadeBound / (float)ShadowInfo->_TextureSize;
            vWorldUnitsPerTexel = XMVectorSet( fWorldUnitsPerTexel, fWorldUnitsPerTexel, 0.0f, 0.0f ); 


		vLightCameraOrthographicMin /= vWorldUnitsPerTexel;
        vLightCameraOrthographicMin = XMVectorFloor( vLightCameraOrthographicMin );
        vLightCameraOrthographicMin *= vWorldUnitsPerTexel;
            
        vLightCameraOrthographicMax /= vWorldUnitsPerTexel;
        vLightCameraOrthographicMax = XMVectorFloor( vLightCameraOrthographicMax );
        vLightCameraOrthographicMax *= vWorldUnitsPerTexel;

		XMMATRIX LightProjection = XMMatrixOrthographicOffCenterRH( 
			XMVectorGetX( vLightCameraOrthographicMin )
			,  XMVectorGetX( vLightCameraOrthographicMax )
			, XMVectorGetY(vLightCameraOrthographicMin)
			, XMVectorGetY(vLightCameraOrthographicMax)
			, -XMVectorGetZ( vLightSpaceSceneAABBmaxValue )
			, -XMVectorGetZ( vLightSpaceSceneAABBminValue )
			);

		XMStoreFloat4x4(&ShadowInfo->_ShadowViewMat, LightView);
		XMStoreFloat4x4(&ShadowInfo->_ShadowProjectionMat, LightProjection);

		SET_RASTERIZER_STATE(RS_SHADOWMAP);

		for(unsigned int i=0;i<_StaticMeshArray.size();i++)
		{
			_GBufferDrawer->DrawStaticMesh(_StaticMeshArray[i], LightView, LightProjection);
		}

		if(_GSkeletalMeshComponent)
		{
			for(unsigned int i=0;i<_GSkeletalMeshComponent->_RenderDataArray.size();i++)
			{
				_GBufferDrawer->DrawSkeletalMeshData(_GSkeletalMeshComponent->_RenderDataArray[i], LightView, LightProjection);
			}
		}

		SET_RASTERIZER_STATE(RS_NORMAL);

		_ImmediateContext->RSSetViewports( nViewPorts, vpOld );
	}
}

void Engine::RenderDeferredShadow()
{

	ID3D11RenderTargetView* aRTViewsLit[ 1] = { _DeferredShadowTexture->GetRTV() };
	_ImmediateContext->OMSetRenderTargets( 1, aRTViewsLit, NULL);     

	//float ShadowClearColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
	float ShadowClearColor[4] = { 1.f, 1.f, 1.f, 1.f }; //red,green,blue,alpha
	_ImmediateContext->ClearRenderTargetView( _DeferredShadowTexture->GetRTV() , ShadowClearColor );

	SET_DEPTHSTENCIL_STATE(DS_LIGHTING_PASS);
	SET_BLEND_STATE(BS_SHADOW);
	SET_PS_SAMPLER(0, SS_LINEAR);
	SET_PS_SAMPLER(1, SS_SHADOW);

	for(unsigned int i=0;i<_CascadeArray.size();i++)
	{
		ShadowCascadeInfo* ShadowInfo = _CascadeArray[i];
		if(ShadowInfo->_bEnabled == false) continue;
		ID3D11ShaderResourceView* aSRVVis[2] = {_DepthTexture->GetSRV(), ShadowInfo->_ShadowDepthTexture->GetSRV()};
		_ImmediateContext->PSSetShaderResources( 0, 2, aSRVVis );
	
	
		_DeferredShadowPS->SetShaderParameter(ShadowInfo);
		DrawFullScreenQuad11(_DeferredShadowPS->GetPixelShader(), _Width, _Height);
	}

	SET_PS_SAMPLER(0, SS_LINEAR);
}

float Engine::_GetTimeSeconds()
{
	LARGE_INTEGER CurrentTime;
	QueryPerformanceCounter(&CurrentTime);
	return (float)(CurrentTime.QuadPart)/(float)_Freq.QuadPart;
}


ShadowCascadeInfo::ShadowCascadeInfo( float ViewNear, float ViewFar, float TextureSize )
	:_ViewNear(ViewNear)
	,_ViewFar(ViewFar)
	,_TextureSize(TextureSize)
{
	CD3D11_TEXTURE2D_DESC ShadowDescDepthTex(DXGI_FORMAT_R24G8_TYPELESS, (UINT)TextureSize, (UINT)TextureSize, 1, 1, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE);
	CD3D11_DEPTH_STENCIL_VIEW_DESC  ShadowDescDSV(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT, 0, 0, 0,0) ;
	CD3D11_SHADER_RESOURCE_VIEW_DESC ShadowDescDepthSRV(D3D11_SRV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	_ShadowDepthTexture = new TextureDepth2D(ShadowDescDepthTex, ShadowDescDSV, ShadowDescDepthSRV);
	_bEnabled = true;
}

ShadowCascadeInfo::~ShadowCascadeInfo()
{
	if(_ShadowDepthTexture) delete _ShadowDepthTexture;
}
