#include <cassert>
#include "Engine.h"
#include "Util.h"
#include "Camera.h"
#include "DeferredShadowPixelShader.h"


DeferredShadowPixelShader::DeferredShadowPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
	:PixelShader(szFileName, szFuncName , pDefines)
{
	CreateConstantBuffer<ShaderConstant>();
}


DeferredShadowPixelShader::~DeferredShadowPixelShader(void)
{
}

void DeferredShadowPixelShader::SetShaderParameter(ShadowCascadeInfo* ShadowInfo)
{
	ShaderConstant _SC;

	float Near = GEngine->_CurrentCamera->GetNear();
	float Far = GEngine->_CurrentCamera->GetFar();
	_SC.Projection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
	_SC.ProjectionParams.x = Far/(Far - Near);
	_SC.ProjectionParams.y = Near/(Near - Far);
	_SC.ProjectionParams.z = Far;
	_SC.ViewportParams.x = (float)GEngine->_Width;
	_SC.ViewportParams.y = (float)GEngine->_Height;
	_SC.ViewportParams.z = (float)ShadowInfo->_TextureSize;

	XMVECTOR Det;
	XMMATRIX InvViewMatrix = XMMatrixInverse(&Det, XMLoadFloat4x4(&GEngine->_ViewMat));
	_SC.ShadowMatrix = XMMatrixTranspose(InvViewMatrix * XMLoadFloat4x4(&ShadowInfo->_ShadowViewMat) * XMLoadFloat4x4(&ShadowInfo->_ShadowProjectionMat));

	GEngine->_ImmediateContext->UpdateSubresource( _ConstantBuffer, 0, NULL, &_SC, 0, 0 );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &_ConstantBuffer );

}

