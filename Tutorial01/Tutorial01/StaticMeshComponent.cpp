#include "StaticMeshComponent.h"
#include "StaticMesh.h"
#include "MathUtil.h"
StaticMeshComponent::StaticMeshComponent(void)
	:_LocalMat(XMMatrixIdentity())
	,_AABBMin(XMFLOAT3(FLOAT_MAX, FLOAT_MAX, FLOAT_MAX))
	,_AABBMax(XMFLOAT3(-FLOAT_MAX, -FLOAT_MAX, -FLOAT_MAX))
{
}


StaticMeshComponent::~StaticMeshComponent(void)
{
}

void StaticMeshComponent::AddStaticMesh( StaticMesh* Mesh )
{
	_StaticMeshArray.push_back(Mesh);


	for(int i=0;i<Mesh->_PositionArray.size();i++)
	{
		XMFLOAT3& Pos = Mesh->_PositionArray[i];
		_AABBMax.x = Math::Max<float>(_AABBMax.x, Pos.x);
		_AABBMax.y = Math::Max<float>(_AABBMax.y, Pos.y);
		_AABBMax.z = Math::Max<float>(_AABBMax.z, Pos.z);

		_AABBMin.x = Math::Min<float>(_AABBMin.x, Pos.x);
		_AABBMin.y = Math::Min<float>(_AABBMin.y, Pos.y);
		_AABBMin.z = Math::Min<float>(_AABBMin.z, Pos.z);
	}
}

