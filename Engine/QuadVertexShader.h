#pragma once
#include "vertexshader.h"
class QuadVertexShader :
	public VertexShader
{
public:
	QuadVertexShader(char* szFileName, char* szFuncName, const D3D11_INPUT_ELEMENT_DESC* layout, int numLayout);
	virtual ~QuadVertexShader(void);
};

