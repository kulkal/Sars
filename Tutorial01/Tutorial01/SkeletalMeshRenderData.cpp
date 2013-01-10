#include <d3d11.h>
#include <d3dx11.h>
#include <cassert>
#include <xnamath.h>
#include "SkeletalMeshRenderData.h"
#include "SkeletalMeshComponent.h"
#include "Skeleton.h"
#include "Util.h"
#include "Engine.h"

SkeletalMeshRenderData::SkeletalMeshRenderData(SkeletalMesh* InSkeletalMesh, SkeletalMeshComponent* InSkeletalMeshComponent )
	:_SkeletalMesh(InSkeletalMesh)
	,_SkeletalMeshComponent(InSkeletalMeshComponent)
	,_BoneMatricesBuffer(NULL)
	,_BoneMatricesBufferRV(NULL)
	,_BoneMatrices(NULL)
{
	if( _BoneMatrices == NULL)
		_BoneMatrices = new XMFLOAT4X4[_SkeletalMesh->_NumBone];

	if(_BoneMatricesBuffer == NULL)
	{

		HRESULT hr;
		D3D11_BUFFER_DESC bdc;
		ZeroMemory( &bdc, sizeof(bdc) );
		bdc.Usage = D3D11_USAGE_DYNAMIC;
		bdc.ByteWidth = _SkeletalMesh->_NumBone* sizeof(XMFLOAT4X4);
		bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bdc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_BoneMatricesBuffer );
		if( FAILED( hr ) )
			assert(false);

		SetD3DResourceDebugName("_BoneMatricesBuffer", _BoneMatricesBuffer);


		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
		SRVDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		SRVDesc.Buffer.ElementOffset = 0;
		SRVDesc.Buffer.ElementWidth = _SkeletalMesh->_NumBone * 4;
		hr = GEngine->_Device->CreateShaderResourceView( _BoneMatricesBuffer, &SRVDesc, &_BoneMatricesBufferRV );
		if( FAILED( hr ) )
			assert(false);

		SetD3DResourceDebugName("_BoneMatricesBufferRV", _BoneMatricesBufferRV);
	}
}


void SkeletalMeshRenderData::UpdateBoneMatrices()
{
	// select used bones
	for(int i=0;i<_SkeletalMesh->_NumBone;i++)
	{
		XMFLOAT3 BonePos, BoneDiff;
		int SkeletonIndex = _SkeletalMesh->_RequiredBoneArray[i];
		
		_BoneMatrices[i] = _SkeletalMeshComponent->_BoneWorld[SkeletonIndex];
	}

	// update buffer
	D3D11_MAPPED_SUBRESOURCE MSR;
	GEngine->_ImmediateContext->Map( _BoneMatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MSR );
	XMFLOAT4X4* pMatrices = (XMFLOAT4X4*)MSR.pData;

	for( int i = 0; i < _SkeletalMesh->_NumBone; i++ )
	{
		pMatrices[i] = _BoneMatrices[i];
	}

	GEngine->_ImmediateContext->Unmap( _BoneMatricesBuffer, 0 );
}

SkeletalMeshRenderData::~SkeletalMeshRenderData(void)
{
	if(_BoneMatricesBuffer)_BoneMatricesBuffer->Release();
	if(_BoneMatricesBufferRV)_BoneMatricesBufferRV->Release();
	if(_BoneMatrices) delete[] _BoneMatrices;
	
}
