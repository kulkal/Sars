#pragma once
#include "lightcomponent.h"

struct DeferredPointPSCBStruct
{
	XMFLOAT4 vLightPos;
	XMFLOAT4 vLightColor;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 ProjectionParams;
	XMFLOAT4 ViewportParams;
};

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
