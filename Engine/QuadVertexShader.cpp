#include "QuadVertexShader.h"


QuadVertexShader::QuadVertexShader(char* szFileName, char* szFuncName, const D3D11_INPUT_ELEMENT_DESC* layout, int numLayout)
	:VertexShader(szFileName, szFuncName, layout, numLayout)
{
}


QuadVertexShader::~QuadVertexShader(void)
{
}
