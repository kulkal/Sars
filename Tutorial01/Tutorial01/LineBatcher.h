#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <string>
#include <vector>
struct LineVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Color;
};
class LineBatcher
{
	ID3D11Buffer*           _VertexBuffer;
	ID3D11Buffer*           _IndexBuffer;

	ID3D11InputLayout*      _VertexLayout;
	ID3D11VertexShader*     _VertexShader;
	ID3D11PixelShader*      _PixelShader;
	ID3D11Buffer*           _ConstantBuffer;

	std::vector<LineVertex> _PositionArray;
	std::vector<WORD>	_IndexArray;
public:
	void InitDevice();
	void BeginLine();
	void AddLine(XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 Color = XMFLOAT3(1.f, 0.f, 0.f));
	void UpdateBuffer();
	void Draw();
	LineBatcher(void);
	virtual ~LineBatcher(void);
};

