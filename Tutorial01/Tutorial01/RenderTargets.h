#pragma once
#include "Engine.h"
class RenderTargets
{
	ID3D11Texture2D* _BackBuffer;
	ID3D11RenderTargetView* _BackBufferView;

	ID3D11Texture2D* _NormalBuffer;
	ID3D11RenderTargetView* _NormalBufferView;

	ID3D11Texture2D* _DiffuseColorBuffer;
	ID3D11RenderTargetView* _DiffuseColorBufferView;

	ID3D11Texture2D*        _DepthStencil;
	ID3D11DepthStencilView* _DepthStencilView;
public:
	RenderTargets(void);
	~RenderTargets(void);
};

