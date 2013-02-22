#pragma once
#include "pixelshader.h"

struct ShadowCascadeInfo;

class DeferredShadowPixelShader :
	public PixelShader
{
	struct ShaderConstant
	{
		XMMATRIX Projection;
		XMFLOAT4 ProjectionParams;
		XMFLOAT4 ViewportParams;
		XMMATRIX ShadowMatrix;
	};
public:
	void SetShaderParameter(ShadowCascadeInfo* ShadowInfo);

	DeferredShadowPixelShader( char* szFileName, char* szFuncName, D3D10_SHADER_MACRO* pDefines = NULL);
	virtual ~DeferredShadowPixelShader(void);
};

