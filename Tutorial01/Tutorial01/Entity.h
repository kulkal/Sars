#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <vector>

#include "BaseObject.h"

class BaseComponent;

class Entity :
	public BaseObject
{
	XMFLOAT3 Location;
	//XMFLOAT4 Rotation;

	std::vector<BaseComponent*> Components;
public:
	Entity(void);
	virtual ~Entity(void);
};

