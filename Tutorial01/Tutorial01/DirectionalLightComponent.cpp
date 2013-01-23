#include "DirectionalLightComponent.h"
#include "Engine.h"

struct DeferredDirPSCBStruct
{
	XMFLOAT4 vLightDir;
	XMFLOAT4 vLightColor;
};

DirectionalLightComponent::DirectionalLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightDir)
	:LightComponent(LightColor)
	,_LightDirection(LightDir)
{
}


DirectionalLightComponent::~DirectionalLightComponent(void)
{
}

void DirectionalLightComponent::RenderLightDeferred()
{
	DeferredDirPSCBStruct cb;
	XMVECTOR LightDirParam = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&_LightDirection), (XMLoadFloat4x4(&GEngine->_ViewMat) ) ) );
	XMStoreFloat4(&cb.vLightDir, LightDirParam);
	memcpy(&cb.vLightColor, &_LightColor, sizeof(XMFLOAT4));

	GEngine->_ImmediateContext->UpdateSubresource( GEngine->_DeferredDirPSCB, 0, NULL, &cb, 0, 0 );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &GEngine->_DeferredDirPSCB );
	GEngine->DrawFullScreenQuad11(GEngine->_DeferredDirPS, GEngine->_Width, GEngine->_Height);
}