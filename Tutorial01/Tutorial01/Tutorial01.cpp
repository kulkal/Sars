//--------------------------------------------------------------------------------------
// File: Tutorial01.cpp
//
// This application demonstrates creating a Direct3D 11 device
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>



#include <windows.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <d3dcompiler.h>

#include "resource.h"

//


#include "FbxFileImporter.h"

#include "Engine.h"
#include "SimpleDrawingPolicy.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshComponent.h"
#include "LineBatcher.h"
#include "AnimationClip.h"
#include "GBufferDrawingPolicy.h"
//#include "vld.h"

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = NULL;
HWND                    g_hWnd = NULL;
//
XMMATRIX                g_View;


ID3D11ShaderResourceView*           g_pTextureRV = NULL;

XMMATRIX                g_Projection;
XMMATRIX                g_World;
XMMATRIX                g_World2;
std::vector<StaticMesh*> StaticMeshArray;
std::vector<SkeletalMesh*> SkeletalMeshArray;
std::vector<AnimationClip*> AnimClipArray;
Skeleton* GSkeleton;
SkeletonPose* GPose;
SkeletalMeshComponent* GSkeletalMeshComponent;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
HRESULT InitDevice();
void CleanupDevice();
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
void Render();


HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
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
    hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
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

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
	
	GEngine = new Engine;
    
	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
			GEngine->Tick();
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 640, 480 };
    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"TutorialWindowClass", L"Direct3D 11 Tutorial 1: Direct3D 11 Basics", WS_OVERLAPPEDWINDOW,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
                           NULL );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

	GEngine->_hWnd = g_hWnd;

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
	
	GEngine->InitDevice();
	
	XMFLOAT4 EyeVal = XMFLOAT4( 0, 220.f, 250.f, 0.0f );
	XMFLOAT4 AtVal = XMFLOAT4( 0.0f, 1.0f, 0.0f, 0.0f );
	XMFLOAT4 UpVal = XMFLOAT4( 0.0f, 1.0f, 0.0f, 0.0f );

    // Initialize the view matrix
	XMVECTOR Eye = XMLoadFloat4(&EyeVal);
	XMVECTOR At = XMLoadFloat4(&AtVal);
	XMVECTOR Up = XMLoadFloat4(&UpVal);
	

	g_View = XMMatrixLookAtRH( Eye, At, Up );
	g_Projection = XMMatrixPerspectiveFovRH( XM_PIDIV2, GEngine->_Width / (FLOAT)GEngine->_Height, 0.01f, 2000 );
	g_World = XMMatrixIdentity();

	XMStoreFloat4x4(&GEngine->_ViewMat, g_View);
	XMStoreFloat4x4(&GEngine->_ProjectionMat, g_Projection);


	HRESULT hr;
	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFile( GEngine->_Device, L"seafloor.dds", NULL, NULL, &g_pTextureRV, NULL );
	if( FAILED( hr ) )
		return hr;

	GSkeletalMeshComponent = new SkeletalMeshComponent;
	GSkeleton = new Skeleton;
	GPose = new SkeletonPose;

	FbxFileImporter FbxImporterObj("humanoid.fbx");
	//FbxFileImporter FbxImporterObj("box_skin.fbx");
	//FbxImporterObj.ImportStaticMesh(StaticMeshArray);

	FbxImporterObj.ImportSkeletalMesh(SkeletalMeshArray);

	for(unsigned int i=0;i<SkeletalMeshArray.size();i++)
	{
		GSkeletalMeshComponent->AddSkeletalMesh(SkeletalMeshArray[i]);
	}
	FbxImporterObj.ImportSkeleton(&GSkeleton, &GPose);
	//FbxImporterObj.ImportAnimClip(AnimClipArray);

	GSkeletalMeshComponent->SetSkeleton(GSkeleton);
	GSkeletalMeshComponent->SetCurrentPose(GPose);

	GEngine->Tick();

	//GSkeletalMeshComponent->PlayAnim(AnimClipArray[1], 0, 1.f);

	FbxFileImporter FbxImporterObj2("other.fbx");
	FbxImporterObj2.ImportStaticMesh(StaticMeshArray);

	//GEngine->InitDevice();

    return S_OK;
}

struct SCREEN_VERTEX
{
	XMFLOAT4 pos;
	XMFLOAT2 tex;
};
ID3D11Buffer*               g_pScreenQuadVB = NULL;
ID3D11InputLayout*          g_pQuadLayout = NULL;
ID3D11VertexShader*         g_pQuadVS = NULL;

void Temp()
{
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
	 pd3dDevice->CreateBuffer( &vbdesc, &InitData, &g_pScreenQuadVB ) ;

	 ID3DBlob* pBlob = NULL;
	 CompileShaderFromFile( L"FinalPass.hlsl", "QuadVS", "vs_4_0", &pBlob ) ;
	 pd3dDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &g_pQuadVS ) ;
	 SetD3DResourceDebugName("QuadVS", g_pQuadVS);

	const D3D11_INPUT_ELEMENT_DESC quadlayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	pd3dDevice->CreateInputLayout( quadlayout, 2, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &g_pQuadLayout ) ;
	if(pBlob) pBlob->Release();
	SetD3DResourceDebugName("Quad", g_pQuadLayout);

}

void DrawFullScreenQuad11( ID3D11DeviceContext* pd3dImmediateContext, 
	ID3D11PixelShader* pPS,
	UINT Width, UINT Height )
{
	// Save the old viewport
	D3D11_VIEWPORT vpOld[D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX];
	UINT nViewPorts = 1;
	pd3dImmediateContext->RSGetViewports( &nViewPorts, vpOld );

	// Setup the viewport to match the backbuffer
	D3D11_VIEWPORT vp;
	vp.Width = (float)Width;
	vp.Height = (float)Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pd3dImmediateContext->RSSetViewports( 1, &vp );

	UINT strides = sizeof( SCREEN_VERTEX );
	UINT offsets = 0;
	ID3D11Buffer* pBuffers[1] = { g_pScreenQuadVB };

	pd3dImmediateContext->IASetInputLayout( g_pQuadLayout );
	pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &strides, &offsets );
	pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	pd3dImmediateContext->VSSetShader( g_pQuadVS, NULL, 0 );
	pd3dImmediateContext->PSSetShader( pPS, NULL, 0 );
	pd3dImmediateContext->Draw( 4, 0 );

	// Restore the Old viewport
	pd3dImmediateContext->RSSetViewports( nViewPorts, vpOld );
}

//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Render()
{
	GEngine->BeginRendering();


	// Update our time
    static float t = 0.0f;
   
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();
        if( dwTimeStart == 0 )
            dwTimeStart = dwTimeCur;
        t = ( dwTimeCur - dwTimeStart ) / 1000.0f;
    }

    //
    // Animate the cube
    //
	g_World = XMMatrixRotationY( t );

	// Setup our lighting parameters
	XMFLOAT4 vLightDirs[2] =
	{
		XMFLOAT4( -0.577f, 0.577f, -0.577f, 1.0f ),
		XMFLOAT4( 0.0f, 0.0f, -1.0f, 1.0f ),
	};
	XMFLOAT4 vLightColors[2] =
	{
		XMFLOAT4( 0.7f, 0.7f, 0.7f, 0.7f ),
		XMFLOAT4( 0, 0.0f, 1, 1.0f )
	};

	// Rotate the second light around the origin
	XMMATRIX mRotate = XMMatrixRotationY( -2.0f * t *0.7f);
	XMVECTOR vLightDir = XMLoadFloat4( &vLightDirs[1] );
	vLightDir = XMVector3Transform( vLightDir, mRotate );
	XMStoreFloat4( &vLightDirs[1], vLightDir );



	 // 2nd Cube:  Rotate around origin
    XMMATRIX mSpin = XMMatrixRotationZ( -t );
    XMMATRIX mOrbit = XMMatrixRotationY( -t * 2.0f );
	XMMATRIX mTranslate = XMMatrixTranslation( -4.0f, 0.0f, 0.0f );
	XMMATRIX mScale = XMMatrixScaling( 0.3f, 0.3f, 0.3f );

	g_World2 = mScale * mSpin * mTranslate * mOrbit;

    //
    // Update variables
    //
   

	GEngine->_ImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );


	memcpy(GEngine->_SimpleDrawer->vLightColors, vLightColors, sizeof(XMFLOAT4)*2);
	memcpy(GEngine->_SimpleDrawer->vLightDirs, vLightDirs, sizeof(XMFLOAT4)*2);

	for(unsigned int i=0;i<StaticMeshArray.size();i++)
	{
		//GEngine->_SimpleDrawer->DrawStaticMesh(StaticMeshArray[i]);
	}

	if(GSkeletalMeshComponent)
	{
		GSkeletalMeshComponent->Tick(GEngine->_DeltaSeconds);
		for(unsigned int i=0;i<GSkeletalMeshComponent->_RenderDataArray.size();i++)
		{
		//	GEngine->_SimpleDrawer->DrawSkeletalMeshData(GSkeletalMeshComponent->_RenderDataArray[i]);
		}
	}

	for(unsigned int i=0;i<StaticMeshArray.size();i++)
	{
		GEngine->_GBufferDrawer->DrawStaticMesh(StaticMeshArray[i]);
	}

	if(GSkeletalMeshComponent)
	{
		for(unsigned int i=0;i<GSkeletalMeshComponent->_RenderDataArray.size();i++)
		{
			GEngine->_GBufferDrawer->DrawSkeletalMeshData(GSkeletalMeshComponent->_RenderDataArray[i]);
		}
	}
	
	GEngine->EndRendering();
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
	if( g_pTextureRV ) g_pTextureRV->Release();

	if(GSkeleton) delete GSkeleton;
	if(GPose) delete GPose;
	if(GSkeletalMeshComponent) delete GSkeletalMeshComponent;

	for(unsigned int i=0;i<StaticMeshArray.size();i++)
	{
		StaticMesh* Mesh = StaticMeshArray[i];
		delete Mesh;
	}

	for(unsigned int i=0;i<SkeletalMeshArray.size();i++)
	{
		SkeletalMesh* Mesh = SkeletalMeshArray[i];
		delete Mesh;
	}

	for(unsigned int i=0;i<AnimClipArray.size();i++)
	{
		AnimationClip* Clip = AnimClipArray[i];
		delete Clip;
	}

	if(GEngine) delete GEngine;
}
