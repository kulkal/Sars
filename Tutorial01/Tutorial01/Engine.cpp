#include <cassert>
#include "Engine.h"
#include "SimpleDrawingPolicy.h"
#include "LineBatcher.h"

Engine* GEngine;
Engine::Engine(void)
	:_hWnd(NULL)
	,_Device(NULL)
	,_ImmediateContext(NULL)
	,_DriverType(D3D_DRIVER_TYPE_NULL)
	,_FeatureLevel(D3D_FEATURE_LEVEL_11_0)
	,_SwapChain(NULL)
	,_RenderTargetView(NULL)
	,_DepthStencilView(NULL)
	,_DepthStencilTexture(NULL)
	,_SimpleDrawer(NULL)
	,_LineBatcher(NULL)
	,_TimeSeconds(0.f)
	
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	QueryPerformanceFrequency(&_Freq);
	QueryPerformanceCounter(&_PrevTime);
	//_CrtSetBreakAlloc(2486860);
}

Engine::~Engine(void)
{
	if( _ImmediateContext ) _ImmediateContext->ClearState();
	if( _DepthStencilTexture ) _DepthStencilTexture->Release();
	if( _DepthStencilView ) _DepthStencilView->Release();
	if( _RenderTargetView ) _RenderTargetView->Release();
	if( _SwapChain ) _SwapChain->Release();
	if( _ImmediateContext ) _ImmediateContext->Release();
	if( _Device ) _Device->Release();

	if(_SimpleDrawer) delete _SimpleDrawer;
	if(_LineBatcher) delete _LineBatcher;
	
}

void Engine::InitDevice()
{
	HRESULT hr;
	RECT rc;
	GetClientRect( _hWnd, &rc );
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;

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
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = _SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), ( LPVOID* )&pBackBuffer );
	if( FAILED( hr ) )
		assert(false);

	hr = _Device->CreateRenderTargetView( pBackBuffer, NULL, &_RenderTargetView );
	pBackBuffer->Release();
	if( FAILED( hr ) )
		assert(false);

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory( &descDepth, sizeof(descDepth) );
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = _Device->CreateTexture2D( &descDepth, NULL, &_DepthStencilTexture );
	if( FAILED( hr ) )
		assert(false);

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory( &descDSV, sizeof(descDSV) );
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = _Device->CreateDepthStencilView( _DepthStencilTexture, &descDSV, &_DepthStencilView );
	if( FAILED( hr ) )
		assert(false);

	_ImmediateContext->OMSetRenderTargets( 1, &_RenderTargetView, _DepthStencilView );

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_ImmediateContext->RSSetViewports( 1, &vp );

	_SimpleDrawer = new SimpleDrawingPolicy;
	_LineBatcher = new LineBatcher;
	_LineBatcher->InitDevice();
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

	_PrevTime = CurrentTime;
}

void Engine::BeginRendering()
{
	_LineBatcher->BeginLine();

	// Just clear the backbuffer
	float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f }; //red,green,blue,alpha
	_ImmediateContext->ClearRenderTargetView( _RenderTargetView, ClearColor );
	_ImmediateContext->ClearDepthStencilView( _DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

void Engine::EndRendering()
{
	GEngine->_LineBatcher->Draw();

	_SwapChain->Present( 0, 0 );
}


