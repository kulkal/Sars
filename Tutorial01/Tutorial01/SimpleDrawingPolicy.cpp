#include <cassert>

#include "SimpleDrawingPolicy.h"
#include "StateManager.h"

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
	,RS(NULL)
{
	FileName = "SimpleShader.fx";

	HRESULT hr;
	// Create the constant buffer
	D3D11_BUFFER_DESC bdc;
	ZeroMemory( &bdc, sizeof(bdc) );
	bdc.Usage = D3D11_USAGE_DEFAULT;
	bdc.ByteWidth = sizeof(ConstantBufferStruct);
	bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdc.CPUAccessFlags = 0;
	hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &ConstantBuffer );
	if( FAILED( hr ) )
		assert(false);

	SetD3DResourceDebugName("SimpleDrawingPolicyConstantBuffer", ConstantBuffer);


	D3D11_RASTERIZER_DESC drd =
	{
            D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
            D3D11_CULL_FRONT,//D3D11_CULL_MODE CullMode;
            FALSE, //BOOL FrontCounterClockwise;
            0, //INT DepthBias;
            0.0f,//FLOAT DepthBiasClamp;
            0.0f,//FLOAT SlopeScaledDepthBias;
            TRUE,//BOOL DepthClipEnable;
            FALSE,//BOOL ScissorEnable;
            FALSE,//BOOL MultisampleEnable;
            FALSE//BOOL AntialiasedLineEnable;        
	};

    hr = GEngine->_Device->CreateRasterizerState(&drd, &RS);
    if ( FAILED( hr ) )
		assert(false);

	//SetD3DResourceDebugName("SimpleDrawingPolicyRS", RS);


    GEngine->_ImmediateContext->RSSetState(RS);
}


SimpleDrawingPolicy::~SimpleDrawingPolicy(void)
{
	if(ConstantBuffer) ConstantBuffer->Release();
	if(RS) RS->Release();
}

void SimpleDrawingPolicy::DrawStaticMesh( StaticMesh* pMesh, XMMATRIX& ViewMat, XMMATRIX& ProjectionMat )
{
	XMMATRIX World;

	World = XMMatrixIdentity();
	ConstantBufferStruct cb;
	cb.mWorld = XMMatrixTranspose( World );
	cb.mView = XMMatrixTranspose(  ViewMat );
	cb.mProjection = XMMatrixTranspose( ProjectionMat);
	cb.vLightDir[0] = vLightDirs[0];
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	GEngine->_ImmediateContext->UpdateSubresource( ConstantBuffer, 0, NULL, &cb, 0, 0 );

	ShaderRes* pShaderRes = GetShaderRes(pMesh->_NumTexCoord, StaticVertex);


	pShaderRes->SetShaderRes();

	UINT offset = 0;
	GEngine->_ImmediateContext->IASetVertexBuffers( 0, 1, &pMesh->_VertexBuffer, &pMesh->_VertexStride, &offset );
	GEngine->_ImmediateContext->IASetIndexBuffer( pMesh->_IndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	GEngine->_ImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );


	GEngine->_ImmediateContext->VSSetConstantBuffers( 0, 1, &ConstantBuffer );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &ConstantBuffer );

	SET_PS_SAMPLER(0, SS_LINEAR);

	GEngine->_ImmediateContext->DrawIndexed( pMesh->_NumTriangle*3, 0, 0 );        // 36 vertices needed for 12 triangles in a triangle list
}

void SimpleDrawingPolicy::DrawSkeletalMeshData( SkeletalMeshRenderData* pRenderData, XMMATRIX& ViewMat, XMMATRIX& ProjectionMat )
{
	XMMATRIX World;

	World = XMMatrixIdentity();
	ConstantBufferStruct cb;
	cb.mWorld = XMMatrixTranspose( World );
	cb.mView = XMMatrixTranspose(  ViewMat );
	cb.mProjection = XMMatrixTranspose( ProjectionMat);
	cb.vLightDir[0] = vLightDirs[0];
	cb.vLightDir[1] = vLightDirs[1];
	cb.vLightColor[0] = vLightColors[0];
	cb.vLightColor[1] = vLightColors[1];
	GEngine->_ImmediateContext->UpdateSubresource( ConstantBuffer, 0, NULL, &cb, 0, 0 );

	ShaderRes* pShaderRes = GetShaderRes(pRenderData->_SkeletalMesh->_NumTexCoord, GpuSkinVertex);


	pShaderRes->SetShaderRes();

	UINT offset = 0;
	GEngine->_ImmediateContext->IASetVertexBuffers( 0, 1, &pRenderData->_SkeletalMesh->_VertexBuffer, &pRenderData->_SkeletalMesh->_VertexStride, &offset );
	GEngine->_ImmediateContext->IASetIndexBuffer( pRenderData->_SkeletalMesh->_IndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

	GEngine->_ImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );


	GEngine->_ImmediateContext->VSSetConstantBuffers( 0, 1, &ConstantBuffer );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &ConstantBuffer );

	GEngine->_ImmediateContext->VSSetShaderResources( 0, 1, &pRenderData->_BoneMatricesBufferRV );

	SET_PS_SAMPLER(0, SS_LINEAR);

	GEngine->_ImmediateContext->DrawIndexed( pRenderData->_SkeletalMesh->_NumTriangle*3, 0, 0 );        // 36 vertices needed for 12 triangles in a triangle list
}