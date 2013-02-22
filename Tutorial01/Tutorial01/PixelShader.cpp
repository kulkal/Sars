#include "PixelShader.h"
#include "Engine.h"

PixelShader::PixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
	:_PixelShader(NULL)
{
	_PixelShader = GEngine->CreatePixelShaderSimple(szFileName, szFuncName, pDefines);
}


PixelShader::~PixelShader(void)
{
	if(_PixelShader) _PixelShader->Release();
}

void PixelShader::SetShader()
{
}
