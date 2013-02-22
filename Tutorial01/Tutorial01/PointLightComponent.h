#pragma once
#include "lightcomponent.h"
class Camera;

class PointLightComponent :
	public LightComponent
{
public:
	XMFLOAT3 _LightPos;
	float _LightRange;
public:
	virtual void RenderLightDeferred(Camera* Camera);

	PointLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightPos, float LightRange);
	virtual ~PointLightComponent(void);
};

