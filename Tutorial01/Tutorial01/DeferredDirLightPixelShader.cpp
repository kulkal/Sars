#include "DeferredDirLightPixelShader.h"
#include "DirectionalLightComponent.h"


DeferredDirLightPixelShader::DeferredDirLightPixelShader(char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
	:PixelShader(szFileName, szFuncName , pDefines)
{
	CreateConstantBuffer<ShaderConstant>();
}


DeferredDirLightPixelShader::~DeferredDirLightPixelShader(void)
{
}

void DeferredDirLightPixelShader::SetShaderParameter(DirectionalLightComponent* Light)
{
	ShaderConstant cb;
	XMVECTOR LightDirParam = XMVector3Normalize(XMVector3TransformNormal(XMLoadFloat3(&Light->_LightDirection), (XMLoadFloat4x4(&GEngine->_ViewMat) ) ) );
	XMStoreFloat4(&cb.vLightDir, LightDirParam);
	memcpy(&cb.vLightColor, &Light->_LightColor, sizeof(XMFLOAT4));

	GEngine->_ImmediateContext->UpdateSubresource( _ConstantBuffer, 0, NULL, &cb, 0, 0 );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &_ConstantBuffer );
}
