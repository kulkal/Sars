#pragma once
#include "shader.h"
class VertexShader :
	public Shader
{
	ID3D11VertexShader* _VertexShader;
public:
	VertexShader(void);
	virtual ~VertexShader(void);
};

