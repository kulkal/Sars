#pragma once
#include "texture2d.h"
class TextureDepth2D :
	public Texture2D
{
	ID3D11DepthStencilView* _DepthStencilView;
	ID3D11DepthStencilView *	_ReadOnlyDepthStencilView;
public:
	ID3D11DepthStencilView* GetDepthStencilView(){return _DepthStencilView;}
	ID3D11DepthStencilView* GetReadOnlyDepthStencilView(){return _ReadOnlyDepthStencilView;}


	TextureDepth2D(D3D11_TEXTURE2D_DESC& TexDesc, D3D11_DEPTH_STENCIL_VIEW_DESC& DSVDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& SRVDesc);
	virtual ~TextureDepth2D(void);
};

