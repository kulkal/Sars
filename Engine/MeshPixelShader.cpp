#include "MeshPixelShader.h"


MeshPixelShader::MeshPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines)
	:PixelShader(szFileName, szFuncName , pDefines)
{
}


MeshPixelShader::~MeshPixelShader(void)
{
}
