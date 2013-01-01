#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>

#include "basecomponent.h"

class StaticMesh;

class StaticMeshComponent :
	public BaseComponent
{
	XMMATRIX _LocalMat;
	StaticMesh* _Mesh;
public:
	void SetStaticMesh(StaticMesh* Mesh);
	StaticMesh* GetStaticMesh(){return _Mesh;}

	StaticMeshComponent(void);
	virtual ~StaticMeshComponent(void);
};

