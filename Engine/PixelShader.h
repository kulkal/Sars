#pragma once
#include "shader.h"
class PixelShader :
	public Shader
{
protected:
	ID3D11PixelShader*		_PixelShader;
public:
	void SetShader();
	void SetShaderParameter(){}

	ID3D11PixelShader* GetPixelShader(){return _PixelShader;}
	
	PixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~PixelShader(void);
};

