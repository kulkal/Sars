#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>

#include "basecomponent.h"

class StaticMesh;

class StaticMeshComponent :
	public BaseComponent
{
public:
	XMFLOAT3 _AABBMin;
	XMFLOAT3 _AABBMax;
	XMMATRIX _LocalMat;

	std::vector<StaticMesh*> _StaticMeshArray;

public:
	void AddStaticMesh(StaticMesh* Mesh);

	StaticMeshComponent(void);
	virtual ~StaticMeshComponent(void);
};

