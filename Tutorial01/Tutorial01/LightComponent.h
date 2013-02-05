#pragma once
#include "basecomponent.h"

class Camera;
class LightComponent :
	public BaseComponent
{
protected:
	XMFLOAT4 _LightColor;
public:
	virtual void RenderLightDeferred(Camera* Camera){}

	LightComponent(XMFLOAT4 LightColor);
	virtual ~LightComponent(void);
};

