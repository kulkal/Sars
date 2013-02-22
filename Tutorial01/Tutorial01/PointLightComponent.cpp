#include "PointLightComponent.h"
#include "Engine.h"
#include "Camera.h"
#include "DeferredPointLightPixelShader.h"


PointLightComponent::PointLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightPos, float LightRange)
	:LightComponent(LightColor)
	,_LightPos(LightPos)
	,_LightRange(LightRange)
{
}


PointLightComponent::~PointLightComponent(void)
{
}

void PointLightComponent::RenderLightDeferred(Camera* Camera)
{
	GEngine->_DeferredPointPS->SetShaderParameter(this);
	GEngine->DrawFullScreenQuad11(GEngine->_DeferredPointPS->GetPixelShader(), GEngine->_Width, GEngine->_Height);
}