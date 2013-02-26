#pragma once
#include "lightcomponent.h"

class DirectionalLightComponent :
	public LightComponent
{
public:

	XMFLOAT3 _LightDirection;
public:
	virtual void RenderLightDeferred(Camera* Camera);

	DirectionalLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightDir);
	virtual ~DirectionalLightComponent(void);
};

