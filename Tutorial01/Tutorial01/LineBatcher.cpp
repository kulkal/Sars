#include "LineBatcher.h"
#include "Engine.h"
#include <cassert>

struct LineBatchCB
{
	XMFLOAT4X4 View;
	XMFLOAT4X4 Projection;
};
LineBatcher::LineBatcher(void)
	:
	_VertexBuffer(NULL)
	,_IndexBuffer(NULL)
	,_VertexLayout(NULL)
	,_VertexShader(NULL)
	,_PixelShader(NULL)
	,_ConstantBuffer(NULL)
{
}


LineBatcher::~LineBatcher(void)
{
	if(_VertexBuffer) _VertexBuffer->Release();
	if(_IndexBuffer) _IndexBuffer->Release();
	if(_VertexLayout) _VertexLayout->Release();
	if(_VertexShader) _VertexShader->Release();
	if(_PixelShader) _PixelShader->Release();
	if(_ConstantBuffer)_ConstantBuffer->Release();

}

void LineBatcher::InitDevice()
{
	HRESULT hr;
	// Create the constant buffer
	D3D11_BUFFER_DESC bdc;
	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(LineBatchCB);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_ConstantBuffer );
	if( FAILED( hr ) )
		assert(false);

	// vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( LineVertex ) * _PositionArray.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	InitData.pSysMem = &_PositionArray.at(0);
	hr = GEngine->_Device->CreateBuffer( &bd, &InitData, &_VertexBuffer );
	if( FAILED( hr ) )
	{
		assert(false);
		return;
	}

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( WORD ) * _IndexArray.size();        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = &_IndexArray.at(0);
	hr = GEngine->_Device->CreateBuffer( &bd, &InitData, &_IndexBuffer );
	if( FAILED( hr ) )
	{
		assert(false);
		return;
	}

	// vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = GEngine->CompileShaderFromFile( L"LineShader", NULL, "VS", "vs_4_0", &pVSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	hr = GEngine->_Device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &_VertexShader );
	if( FAILED( hr ) )
	{	

		pVSBlob->Release();
		assert(false);
	}

	// pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = GEngine->CompileShaderFromFile(L"LineShader", NULL, "PS", "ps_4_0", &pPSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	hr = GEngine->_Device->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &_PixelShader );
	pPSBlob->Release();
	if( FAILED( hr ) )
		assert(false);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = GEngine->_Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_VertexLayout );
}


void LineBatcher::AddLine(XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 Color)
{
	LineVertex Vertex1;
	Vertex1.Pos = p1;
	Vertex1.Color = Color;

	LineVertex Vertex2;
	Vertex2.Pos = p2;
	Vertex2.Color = Color;

	_PositionArray.push_back(Vertex1);
	_PositionArray.push_back(Vertex2);
}

void LineBatcher::UpdateBuffer()
{
}

void LineBatcher::BeginLine()
{
	_PositionArray.clear();
}
