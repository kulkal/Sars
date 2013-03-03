#pragma once
#include "shader.h"
class VertexShader :
	public Shader
{
	ID3D11InputLayout*      _VertexLayout;
	ID3D11VertexShader*		_VertexShader;
public:

	ID3D11VertexShader* GetVertexShader(){return _VertexShader;}

	void SetShader();

	VertexShader(char* szFileName, char* szFuncName);
	VertexShader(char* szFileName, char* szFuncName, const D3D11_INPUT_ELEMENT_DESC* layout, int numLayout, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~VertexShader(void);
};

