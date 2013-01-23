#pragma once
#include "basecomponent.h"

class LightComponent :
	public BaseComponent
{
protected:
	XMFLOAT4 _LightColor;
public:
	virtual void RenderLightDeferred(){}

	LightComponent(XMFLOAT4 LightColor);
	virtual ~LightComponent(void);
};

