#pragma once
#include "lightcomponent.h"


struct DeferredDirPSCBStruct
{
	XMFLOAT4 vLightDir;
	XMFLOAT4 vLightColor;
};

class DirectionalLightComponent :
	public LightComponent
{
public:

	XMFLOAT3 _LightDirection;
public:
	virtual void RenderLightDeferred();

	DirectionalLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightDir);
	virtual ~DirectionalLightComponent(void);
};

