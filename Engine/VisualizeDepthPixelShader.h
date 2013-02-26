#pragma once
#include "pixelshader.h"
class VisualizeDepthPixelShader :
	public PixelShader
{
	struct ShaderConstant
	{
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 ProjectionParams;
	};
public:
	void SetShaderParameter();


	VisualizeDepthPixelShader(char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~VisualizeDepthPixelShader(void);
};

