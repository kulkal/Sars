#pragma once
#include <d3d11.h>

class Shader
{
	// std::map<ShaderMapKey, ShaderRes*> ShaderMap;
	// shader constants

	ID3D11InputLayout*		_VertexLayout;

	ID3D11VertexShader*		_VertexShader;
	ID3D11PixelShader*		_PixelShader;
	ID3D11Buffer*           _ConstantBuffer;
public:
	Shader(void);
	virtual ~Shader(void);
};

