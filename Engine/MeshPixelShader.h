#pragma once
#include "pixelshader.h"
class MeshPixelShader :
	public PixelShader
{
public:
	MeshPixelShader( char* szFileName, char* szFuncName);
	virtual ~MeshPixelShader(void);
};

