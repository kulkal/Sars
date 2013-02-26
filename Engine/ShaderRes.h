#pragma once

#include <d3d11.h>

enum EVertexProcessingType
{
	StaticVertex,
	GpuSkinVertex
};



struct ShaderMapKey
{
	int NumTex;
	EVertexProcessingType VertexProcessingType;
	bool operator<(const ShaderMapKey& other) const        
	{             
		if( NumTex < other.NumTex ) return true;
		if( NumTex > other.NumTex ) return false;

		if( VertexProcessingType < other.VertexProcessingType ) return true;
		if( VertexProcessingType > other.VertexProcessingType ) return false;

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