#pragma once

#include <d3d11.h>

enum EVertexProcessingType
{
	StaticVertex,
	GpuSkinVertex
};

enum ENumTexCoord
{
	NumTexCoord0,
	NumTexCoord1,
	NumTexCoord2,
	NumTexCoord3,
	NumTexCoord4,
};

struct ShaderMapKey
{
	ENumTexCoord NumTex;
	EVertexProcessingType VertexProcessingType;
	bool operator<(const ShaderMapKey& other) const        
	{             
		if( NumTex < other.NumTex 
			&& VertexProcessingType < other.VertexProcessingType)
			return true;
		else
			return false;
	};
};

class ShaderRes
{
public:
	ID3D11InputLayout*      VertexLayout;
	ID3D11VertexShader*     VertexShader;
	ID3D11PixelShader*      PixelShader;

	ShaderRes();
	~ShaderRes();

	void SetShaderRes();

	void CreateShader(const char* FileName, ShaderMapKey& SKey);
};