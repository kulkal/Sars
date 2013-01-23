#include "PointLightComponent.h"
#include "Engine.h"

struct DeferredPointPSCBStruct
{
	XMFLOAT4 vLightPos;
	XMFLOAT4 vLightColor;
	XMMATRIX mView;
	XMMATRIX mProjection;
	XMFLOAT4 ProjectionParams;
};

PointLightComponent::PointLightComponent(XMFLOAT4 LightColor, XMFLOAT3 LightPos, float LightRange)
	:LightComponent(LightColor)
	,_LightPos(LightPos)
	,_LightRange(LightRange)
{
}


PointLightComponent::~PointLightComponent(void)
{
}

void PointLightComponent::RenderLightDeferred()
{
	DeferredPointPSCBStruct cbPoint;
	XMVECTOR LightParam = XMVector3TransformCoord(XMLoadFloat3(&_LightPos), (XMLoadFloat4x4(&GEngine->_ViewMat) ) );
	XMStoreFloat4(&cbPoint.vLightPos, LightParam );

	memcpy(&cbPoint.vLightColor, &_LightColor, sizeof(XMFLOAT4));
	cbPoint.vLightColor.w = _LightRange;
	cbPoint.mView = XMMatrixTranspose( XMLoadFloat4x4( &GEngine->_ViewMat ));
	cbPoint.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));

	cbPoint.ProjectionParams.x = GEngine->_Far/(GEngine->_Far - GEngine->_Near);
	cbPoint.ProjectionParams.y = GEngine->_Near/(GEngine->_Near - GEngine->_Far);
	cbPoint.ProjectionParams.z = GEngine->_Far;
	GEngine->_ImmediateContext->UpdateSubresource( GEngine->_DeferredPointPSCB, 0, NULL, &cbPoint, 0, 0 );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &GEngine->_DeferredPointPSCB );
	GEngine->DrawFullScreenQuad11(GEngine->_DeferredPointPS, GEngine->_Width, GEngine->_Height);
}