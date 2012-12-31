#include "SimpleDrawingPolicy.h"
#include "Engine.h"
#include "StaticMesh.h"
#include <cassert>

struct ConstantBufferStruct
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 vLightDir[2];
	XMFLOAT4 vLightColor[2];
};

SimpleDrawingPolicy::SimpleDrawingPolicy(void)
	:ConstantBuffer(NULL)
{
	HRESULT hr;
	ID3DBlob* pVSBlob = NULL;
	hr = GEngine->CompileShaderFromFile( L"SimpleShader.fx", "VS", "vs_4_0", &pVSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	// Create the vertex shader
	hr = GEngine->Device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &VertexShader );
	if( FAILED( hr ) )
	{	
		pVSBlob->Release();
		assert(false);
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = GEngine->Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &VertexLayout );
	pVSBlob->Release();
	if( FAILED( hr ) )
		assert(false);

	// Set the input layout
	GEngine->ImmediateContext->IASetInputLayout( VertexLayout );

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = GEngine->CompileShaderFromFile( L"SimpleShader.fx", "PS", "ps_4_0", &pPSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	// Create the pixel shader
	hr = GEngine->Device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &PixelShader );
	pPSBlob->Release();
	if( FAILED( hr ) )
		assert(false);

	// Create the constant buffer
	D3D11_BUFFER_DESC bdc;
	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(ConstantBufferStruct);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->Device->CreateBuffer( &bdc, NULL, &ConstantBuffer );
	if( FAILED( hr ) )
		assert(false);
}


SimpleDrawingPolicy::~SimpleDrawingPolicy(void)
{
}

void SimpleDrawingPolicy::DrawStaticMesh( StaticMesh* pMesh )
{
	XMMATRIX World;

	World = XMMatrixIdentity();
	ConstantBufferStruct cb;
	cb.mWorld = XMMatrixTranspose( World );
	cb.mView = XMMatrixTranspose( GEngine->ViewMat );
	cb.mProjection = XMMatrixTranspose( GEngine->ProjectionMat );
	cb.vLightDir[0] = vLightDirs[0];
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	GEngine->ImmediateContext->UpdateSubresource( ConstantBuffer, 0, NULL, &cb, 0, 0 );

	GEngine->ImmediateContext->IASetInputLayout( VertexLayout );
	UINT stride = sizeof( NormalVertex );
	UINT offset = 0;
	GEngine->ImmediateContext->IASetVertexBuffers( 0, 1, &pMesh->VertexBuffer, &stride, &offset );
	GEngine->ImmediateContext->IASetIndexBuffer( pMesh->IndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	// Set primitive topology
	GEngine->ImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Render a triangle
	GEngine->ImmediateContext->VSSetShader( VertexShader, NULL, 0 );
	GEngine->ImmediateContext->PSSetShader( PixelShader, NULL, 0 );

	GEngine->ImmediateContext->VSSetConstantBuffers( 0, 1, &ConstantBuffer );
	GEngine->ImmediateContext->PSSetConstantBuffers( 0, 1, &ConstantBuffer );

	//GEngine->ImmediateContext->PSSetShaderResources( 0, 1, &g_pTextureRV );
	//GEngine->ImmediateContext->PSSetSamplers( 0, 1, &g_pSamplerLinear );
	GEngine->ImmediateContext->DrawIndexed( pMesh->NumTriangle*3, 0, 0 );        // 36 vertices needed for 12 triangles in a triangle list
}
