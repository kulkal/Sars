#pragma once
#include "pixelshader.h"

class PointLightComponent;

class DeferredPointLightPixelShader :
	public PixelShader
{
	struct ShaderConstant
	{
		XMFLOAT4 vLightPos;
		XMFLOAT4 vLightColor;
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 ProjectionParams;
		XMFLOAT4 ViewportParams;
	};
public:
	void SetShaderParameter(PointLightComponent* Light);

	DeferredPointLightPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~DeferredPointLightPixelShader(void);
};

