#pragma once
#include <d3d11.h>
#include <d3dx11.h>

class Texture2D
{
protected:
	ID3D11Texture2D*			_Texture;
	ID3D11RenderTargetView*		_RenderTargetView;
	ID3D11ShaderResourceView*	_ShaderResourceView;
public:
	ID3D11Texture2D*			GetTexture(){return _Texture;}
	ID3D11RenderTargetView*		GetRTV(){return _RenderTargetView;}
	ID3D11ShaderResourceView*	GetSRV(){return _ShaderResourceView;}

	Texture2D(D3D11_TEXTURE2D_DESC& TextureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& SRVDesc, bool bCreateRTV );
	Texture2D(ID3D11Texture2D* InTexture, bool bCreateRTV );

	virtual ~Texture2D();
};