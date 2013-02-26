#include "VisualizeSimplePixelShader.h"
#include "Engine.h"
#include "Camera.h"

VisualizeSimplePixelShader::VisualizeSimplePixelShader(char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
	:PixelShader(szFileName, szFuncName , pDefines)
{
}


VisualizeSimplePixelShader::~VisualizeSimplePixelShader(void)
{
}
