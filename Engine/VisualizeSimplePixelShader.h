#pragma once
#include "pixelshader.h"
class VisualizeSimplePixelShader :
	public PixelShader
{
public:
	VisualizeSimplePixelShader(char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~VisualizeSimplePixelShader(void);
};

