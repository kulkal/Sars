#include <cassert>

#include "SimpleDrawingPolicy.h"

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
	,_SamplerLinear(NULL)
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

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = GEngine->_Device->CreateSamplerState( &sampDesc, &_SamplerLinear );
	if( FAILED( hr ) )
		assert(false);

	//SetD3DResourceDebugName("SimpleDrawingPolicy_SamplerLinear", _SamplerLinear);

}


SimpleDrawingPolicy::~SimpleDrawingPolicy(void)
{
	if(ConstantBuffer) ConstantBuffer->Release();
	if(RS) RS->Release();
	if(_SamplerLinear)_SamplerLinear->Release();
}

void SimpleDrawingPolicy::DrawStaticMesh( StaticMesh* pMesh )
{
	XMMATRIX World;

	World = XMMatrixIdentity();
	ConstantBufferStruct cb;
	cb.mWorld = XMMatrixTranspose( World );
	cb.mView = XMMatrixTranspose( XMLoadFloat4x4( &GEngine->_ViewMat ));
	cb.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
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

	GEngine->_ImmediateContext->PSSetSamplers( 0, 1, &_SamplerLinear );


	GEngine->_ImmediateContext->DrawIndexed( pMesh->_NumTriangle*3, 0, 0 );        // 36 vertices needed for 12 triangles in a triangle list
}

void SimpleDrawingPolicy::DrawSkeletalMeshData(SkeletalMeshRenderData* pRenderData) 
{
	XMMATRIX World;

	World = XMMatrixIdentity();
	ConstantBufferStruct cb;
	cb.mWorld = XMMatrixTranspose( World );
	cb.mView = XMMatrixTranspose( XMLoadFloat4x4( &GEngine->_ViewMat ));
	cb.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
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

	GEngine->_ImmediateContext->PSSetSamplers( 0, 1, &_SamplerLinear );
	GEngine->_ImmediateContext->DrawIndexed( pRenderData->_SkeletalMesh->_NumTriangle*3, 0, 0 );        // 36 vertices needed for 12 triangles in a triangle list
}