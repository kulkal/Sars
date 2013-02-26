#include <cassert>
#include "Engine.h"
#include "Util.h"
#include "Camera.h"
#include "DeferredPointLightPixelShader.h"
#include "PointLightComponent.h"


DeferredPointLightPixelShader::DeferredPointLightPixelShader(char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines )
	:PixelShader(szFileName, szFuncName , pDefines)
{
	CreateConstantBuffer<ShaderConstant>();
}


DeferredPointLightPixelShader::~DeferredPointLightPixelShader(void)
{
}

void DeferredPointLightPixelShader::SetShaderParameter(PointLightComponent* Light)
{
	ShaderConstant _SC;

	XMVECTOR LightParam = XMVector3TransformCoord(XMLoadFloat3(&Light->_LightPos), (XMLoadFloat4x4(&GEngine->_ViewMat) ) );
	XMStoreFloat4(&_SC.vLightPos, LightParam );

	memcpy(&_SC.vLightColor, &Light->_LightColor, sizeof(XMFLOAT4));
	_SC.vLightColor.w = Light->_LightRange;
	_SC.mView = XMMatrixTranspose( XMLoadFloat4x4( &GEngine->_ViewMat ));
	_SC.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));

	float Near = GEngine->_CurrentCamera->GetNear();
	float Far = GEngine->_CurrentCamera->GetFar();
	_SC.ProjectionParams.x =Far/(Far - Near);
	_SC.ProjectionParams.y =Near/(Near - Far);
	_SC.ProjectionParams.z =Far;
	_SC.ViewportParams.x = (float)GEngine->_Width;
	_SC.ViewportParams.y = (float)GEngine->_Height;
	GEngine->_ImmediateContext->UpdateSubresource( _ConstantBuffer, 0, NULL, &_SC, 0, 0 );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &_ConstantBuffer );
}