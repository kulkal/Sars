#pragma once
#include "pixelshader.h"

class DirectionalLightComponent;
class DeferredDirLightPixelShader :
	public PixelShader
{
	struct ShaderConstant
	{
		XMFLOAT4 vLightDir;
		XMFLOAT4 vLightColor;
	};
public:
	void SetShaderParameter(DirectionalLightComponent* Light);

	DeferredDirLightPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~DeferredDirLightPixelShader(void);
};

