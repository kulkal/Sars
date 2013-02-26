#include "DirectionalLightComponent.h"
#include "Engine.h"
#include "Camera.h"
#include "DeferredDirLightPixelShader.h"

DirectionalLightComponent::DirectionalLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightDir)
	:LightComponent(LightColor)
	,_LightDirection(LightDir)
{
	XMStoreFloat3(&_LightDirection, XMVector3Normalize(XMLoadFloat3(&LightDir)));
}


DirectionalLightComponent::~DirectionalLightComponent(void)
{
}

void DirectionalLightComponent::RenderLightDeferred(Camera* Camera)
{
	GEngine->_DeferredDirPS->SetShaderParameter(this);
	GEngine->DrawFullScreenQuad11(GEngine->_DeferredDirPS->GetPixelShader(), GEngine->_Width, GEngine->_Height);
}