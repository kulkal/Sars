#include "StaticMeshComponent.h"


StaticMeshComponent::StaticMeshComponent(void)
	:_LocalMat(XMMatrixIdentity()),
	_Mesh(NULL)
{
}


StaticMeshComponent::~StaticMeshComponent(void)
{
}

void StaticMeshComponent::SetStaticMesh( StaticMesh* Mesh )
{
	_Mesh = Mesh;
}

