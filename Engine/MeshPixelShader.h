#pragma once
#include "pixelshader.h"
class MeshPixelShader :
	public PixelShader
{
public:
	MeshPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~MeshPixelShader(void);
};

