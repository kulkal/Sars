#include "VisualizeDepthPixelShader.h"
#include "Engine.h"
#include "Camera.h"

VisualizeDepthPixelShader::VisualizeDepthPixelShader(char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
	:PixelShader(szFileName, szFuncName , pDefines)
{
	CreateConstantBuffer<ShaderConstant>();
}


VisualizeDepthPixelShader::~VisualizeDepthPixelShader(void)
{
}

void VisualizeDepthPixelShader::SetShaderParameter()
{
	float Near = GEngine->_CurrentCamera->GetNear();
	float Far = GEngine->_CurrentCamera->GetFar();
	ShaderConstant cb;
	cb.mView = XMMatrixTranspose( XMLoadFloat4x4( &GEngine->_ViewMat ));
	cb.mProjection = XMMatrixTranspose( XMLoadFloat4x4(&GEngine->_ProjectionMat));
	cb.ProjectionParams.x = Far/(Far - Near);
	cb.ProjectionParams.y = Near/(Near - Far);
	GEngine->_ImmediateContext->UpdateSubresource( _ConstantBuffer, 0, NULL, &cb, 0, 0 );
	GEngine->_ImmediateContext->PSSetConstantBuffers( 0, 1, &_ConstantBuffer );
}
