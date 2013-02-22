#pragma once
#include "shader.h"
class PixelShader :
	public Shader
{
protected:
	ID3D11PixelShader*		_PixelShader;
public:
	PixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	void SetShader();

	ID3D11PixelShader* GetPixelShader(){return _PixelShader;}

	void SetShaderParameter(){}
	virtual ~PixelShader(void);
};

