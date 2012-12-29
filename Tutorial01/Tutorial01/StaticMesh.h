#pragma once

#include <xnamath.h>

#include "baseobject.h"
class StaticMesh :
	public BaseObject
{
	XMFLOAT3* Position;
	XMFLOAT3* Normal;
	XMFLOAT3* TexCoord0;
public:
	StaticMesh(void);
	virtual ~StaticMesh(void);
};

