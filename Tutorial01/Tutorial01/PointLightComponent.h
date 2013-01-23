#pragma once
#include "lightcomponent.h"
class PointLightComponent :
	public LightComponent
{
public:
	XMFLOAT3 _LightPos;
	float _LightRange;
public:
	virtual void RenderLightDeferred();

	PointLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightPos, float LightRange);
	virtual ~PointLightComponent(void);
};

