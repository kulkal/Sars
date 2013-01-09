#include "Engine.h"
#include "SimpleDrawingPolicy.h"
#include "LineBatcher.h"

Engine* GEngine;
Engine::Engine(void)
	:_Device(NULL),
	_ImmediateContext(NULL),
	_SimpleDrawer(NULL)
	//ViewMat(XMMatrixIdentity()),
	//ProjectionMat(XMMatrixIdentity())
	,_LineBatcher(NULL)
{
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	//_CrtSetBreakAlloc(41633);
}

Engine::~Engine(void)
{
	if(_SimpleDrawer) delete _SimpleDrawer;
	if(_LineBatcher) delete _LineBatcher;
	
}

void Engine::InitDevice()
{
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
