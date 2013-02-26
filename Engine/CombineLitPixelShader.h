#pragma once
#include "pixelshader.h"
class CombineLitPixelShader :
	public PixelShader
{
public:
	CombineLitPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~CombineLitPixelShader(void);
};

