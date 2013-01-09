
#include "LineBatcher.h"
#include "Engine.h"

#include <cassert>

struct LineBatchCB
{
	XMMATRIX View;
	XMMATRIX Projection;
};
LineBatcher::LineBatcher(void)
	:
	_VertexBuffer(NULL)
	,_VertexLayout(NULL)
	,_VertexShader(NULL)
	,_PixelShader(NULL)
	,_ConstantBuffer(NULL)
{
}


LineBatcher::~LineBatcher(void)
{
	if(_VertexBuffer) _VertexBuffer->Release();
	if(_VertexLayout) _VertexLayout->Release();
	if(_VertexShader) _VertexShader->Release();
	if(_PixelShader) _PixelShader->Release();
	if(_ConstantBuffer)_ConstantBuffer->Release();
}

void LineBatcher::InitDevice()
{
	_PositionArray.resize(1000);
	_IndexArray.resize(1000);
	for(int i=0;i<_IndexArray.size();i++)
	{
		_IndexArray[i] = i;
	}
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

	SetD3DResourceDebugName("LineBatcherConstantBuffer", _ConstantBuffer);

	// vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DYNAMIC;
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

	SetD3DResourceDebugName("LineBatcher_VertexBuffer", _VertexBuffer);


	// vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = GEngine->CompileShaderFromFile( L"LineShader.fx", NULL, "VS", "vs_4_0", &pVSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	hr = GEngine->_Device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &_VertexShader );
	
	if( FAILED( hr ) )
		assert(false);
	SetD3DResourceDebugName("LineBatcher_VertexShader", _VertexShader);


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE( layout );

	// Create the input layout
	hr = GEngine->_Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_VertexLayout );
	
	pVSBlob->Release();

	if( FAILED( hr ) )
		assert(false);

	// pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = GEngine->CompileShaderFromFile(L"LineShader.fx", NULL, "PS", "ps_4_0", &pPSBlob );
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

	SetD3DResourceDebugName("LineBatcher_PixelShader", _PixelShader);
}


void LineBatcher::AddLine(XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 Color1, XMFLOAT3 Color2)
{
	float DistSqr = p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
	if(DistSqr < 0.0001f)
		return;
	LineVertex Vertex1;
	Vertex1.Pos = p1;
	Vertex1.Color = Color1;

	LineVertex Vertex2;
	Vertex2.Pos = p2;
	Vertex2.Color = Color2;

	_PositionArray.push_back(Vertex1);
	_PositionArray.push_back(Vertex2);
}

void LineBatcher::UpdateBuffer()
{
	if(_PositionArray.size() == 0) return;
	HRESULT hr = S_OK;
	D3D11_MAPPED_SUBRESOURCE MSR;
	GEngine->_ImmediateContext->Map( _VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MSR );
	LineVertex* pVertices = (LineVertex*)MSR.pData;

	memcpy(pVertices, &_PositionArray.at(0), sizeof(LineVertex)*_PositionArray.size());

	GEngine->_ImmediateContext->Unmap( _VertexBuffer, 0 );
	
	LineBatchCB cb;
	cb.View = XMMatrixTranspose( XMLoadFloat4x4( &GEngine->_ViewMat ));
	cb.Projection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
	GEngine->_ImmediateContext->UpdateSubresource( _ConstantBuffer, 0, NULL, &cb, 0, 0 );
}

void LineBatcher::BeginLine()
{
	_PositionArray.clear();
	_IndexArray.clear();
}

void LineBatcher::Draw()
{
	UpdateBuffer();

	GEngine->_ImmediateContext->IASetInputLayout( _VertexLayout );
	GEngine->_ImmediateContext->VSSetShader( _VertexShader, NULL, 0 );
	GEngine->_ImmediateContext->PSSetShader( _PixelShader, NULL, 0 );

	UINT _VertexStride = sizeof(LineVertex);
	UINT offset = 0;
	GEngine->_ImmediateContext->IASetVertexBuffers( 0, 1, &_VertexBuffer, &_VertexStride, &offset );

	GEngine->_ImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );


	GEngine->_ImmediateContext->VSSetConstantBuffers( 0, 1, &_ConstantBuffer );
	//GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &_ConstantBuffer );

	//GEngine->_ImmediateContext->PSSetSamplers( 0, 1, &_SamplerLinear );

	int NumIndex = _PositionArray.size();
	//GEngine->_ImmediateContext->DrawIndexed(NumIndex , 0, 0 );    
	GEngine->_ImmediateContext->Draw(_PositionArray.size(), 0);

}