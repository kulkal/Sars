#include "SkeletalMeshComponent.h"


SkeletalMeshComponent::SkeletalMeshComponent(void)
	:
	_VertexBuffer(NULL)
	,_IndexBuffer(NULL)
	,_BoneMatricesBuffer(NULL)
	,_BoneMatricesBufferRV(NULL)
	,_BoneMatrices(NULL)
	,_BoneWorld(NULL)
	,_SkeletalMesh(NULL)
{
}


SkeletalMeshComponent::~SkeletalMeshComponent(void)
{
	if(_VertexBuffer) _VertexBuffer->Release();
	if(_IndexBuffer) _IndexBuffer->Release();
	if(_BoneMatricesBuffer) _BoneMatricesBuffer->Release();
	if(_BoneMatricesBufferRV) _BoneMatricesBufferRV->Release();
	if(_BoneMatrices) delete[] _BoneMatrices;
	if(_BoneWorld) delete[] _BoneWorld;
	if(_SkeletalMesh) delete _SkeletalMesh;
}

void SkeletalMeshComponent::UpdateBoneMatrices()
{
	if( _BoneMatrices == NULL)
		_BoneMatrices = new XMFLOAT4X4[_Skeleton->_JointCount];
	if( _BoneWorld == NULL)
		_BoneWorld = new XMFLOAT4X4[_Skeleton->_JointCount];
	if(_BoneMatricesBuffer == NULL)
	{

		HRESULT hr;
		D3D11_BUFFER_DESC bdc;
		ZeroMemory( &bdc, sizeof(bdc) );
		bdc.Usage = D3D11_USAGE_DYNAMIC;
		bdc.ByteWidth = _NumBone* sizeof(XMFLOAT4X4);
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
		SRVDesc.Buffer.ElementWidth = _NumBone * 4;
		hr = GEngine->_Device->CreateShaderResourceView( _BoneMatricesBuffer, &SRVDesc, &_BoneMatricesBufferRV );
		if( FAILED( hr ) )
			assert(false);

	}

	for(int i=0;i<_Skeleton->_JointCount;i++)
	{
		XMMATRIX MatParent;

		SkeletonJoint& RefPose = _Skeleton->_Joints[i];

		XMFLOAT4X4& RefInvF = _Skeleton->_Joints[i]._InvRefPose;
		XMVECTOR Det;
		XMMATRIX RefMat = XMMatrixInverse(&Det, XMLoadFloat4x4(&RefInvF));


		XMFLOAT4X4& RefInvParentF = _Skeleton->_Joints[RefPose._ParentIndex]._InvRefPose;
		XMMATRIX RefMatParent = XMMatrixInverse(&Det, XMLoadFloat4x4(&RefInvParentF));

		if( RefPose._ParentIndex >= 0)
			GEngine->_LineBatcher->AddLine(XMFLOAT3(RefMat._41, RefMat._42, RefMat._43), XMFLOAT3(RefMatParent._41, RefMatParent._42, RefMatParent._43), XMFLOAT3(0, 1, 0), XMFLOAT3(0, 0, 1));
	}


	for(int i=0;i<_Skeleton->_JointCount;i++)
	{
		SkeletonJoint& RefPose = _Skeleton->_Joints[i];
		JointPose& LocalPose = _Pose->_LocalPoseArray[i];
		XMMATRIX MatScale = XMMatrixScaling(LocalPose._Scale.x, LocalPose._Scale.y, LocalPose._Scale.z);
		XMVECTOR QuatVec = XMLoadFloat4(&LocalPose._Rot);
		XMMATRIX MatRot = XMMatrixRotationQuaternion(QuatVec);
		XMMATRIX MatTrans = XMMatrixTranslation(LocalPose._Trans.x, LocalPose._Trans.y, LocalPose._Trans.z);

		
		XMMATRIX MatBone;
		if(RefPose._ParentIndex < 0) // root
		{
			MatBone = XMMatrixIdentity();
			MatBone = XMMatrixMultiply(MatScale, MatRot);
			MatBone = XMMatrixMultiply(MatBone, MatTrans);

			XMStoreFloat4x4(&_BoneWorld[i], MatBone);
		}
		else
		{
			XMMATRIX MatParent;
			MatParent = XMLoadFloat4x4(&_BoneWorld[RefPose._ParentIndex]);

			MatBone = XMMatrixIdentity();
			MatBone = XMMatrixMultiply(MatScale, MatRot);
			MatBone = XMMatrixMultiply(MatBone, MatTrans);
			MatBone = XMMatrixMultiply(MatBone, MatParent);

			XMStoreFloat4x4(&_BoneWorld[i], MatBone);

			GEngine->_LineBatcher->AddLine(XMFLOAT3(MatParent._41, MatParent._42, MatParent._43), XMFLOAT3(MatBone._41, MatBone._42, MatBone._43), XMFLOAT3(1, 0, 0), XMFLOAT3(1, 0, 0));
		}
	}

	for(int i=0;i<_NumBone;i++)
	{
		XMFLOAT3 BonePos, BoneDiff;
		int SkeletonIndex = _RequiredBoneArray[i];
		XMFLOAT4X4& RefInvF = _Skeleton->_Joints[SkeletonIndex]._InvRefPose;

		XMMATRIX RefInv;
		RefInv = XMLoadFloat4x4(&RefInvF);

		XMMATRIX World;
		World = XMLoadFloat4x4(&_BoneWorld[SkeletonIndex]);
	
		XMMATRIX MatBone = XMMatrixIdentity();
		MatBone = XMMatrixMultiply(RefInv, World);
		XMStoreFloat4x4(&_BoneMatrices[i], MatBone);
	}

	D3D11_MAPPED_SUBRESOURCE MSR;
	GEngine->_ImmediateContext->Map( _BoneMatricesBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MSR );
	XMFLOAT4X4* pMatrices = (XMFLOAT4X4*)MSR.pData;

	for( unsigned int i = 0; i < _NumBone; i++ )
	{
		pMatrices[i] = _BoneMatrices[i];
	}

	GEngine->_ImmediateContext->Unmap( _BoneMatricesBuffer, 0 );
}
