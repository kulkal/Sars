#include <cassert>
#include "Texture2D.h"
#include "Engine.h"

Texture2D::Texture2D(D3D11_TEXTURE2D_DESC& TextureDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& SRVDesc, bool bCreateRTV)
	:_Texture(NULL)
	,_RenderTargetView(NULL)
	,_ShaderResourceView(NULL)
{
	HRESULT hr;
	hr = GEngine->_Device->CreateTexture2D( &TextureDesc, NULL, &_Texture );
	if( FAILED( hr ) )
		assert(false);

	if(bCreateRTV)
	{
		hr = GEngine->_Device->CreateRenderTargetView( _Texture, NULL, &_RenderTargetView );
		if( FAILED( hr ) )
			assert(false);
	}

	hr = GEngine->_Device->CreateShaderResourceView(_Texture, &SRVDesc, &_ShaderResourceView);
	if( FAILED( hr ) )
		assert(false);

}

Texture2D::Texture2D(ID3D11Texture2D* InTexture, bool bCreateRTV )
	:_Texture(InTexture)
	,_RenderTargetView(NULL)
	,_ShaderResourceView(NULL)
{
	HRESULT hr;

	if(bCreateRTV)
	{
		hr = GEngine->_Device->CreateRenderTargetView( _Texture, NULL, &_RenderTargetView );
		if( FAILED( hr ) )
			assert(false);
	}
}


Texture2D::~Texture2D(void)
{
	if(_Texture) _Texture->Release();
	if(_RenderTargetView) _RenderTargetView->Release();
	if(_ShaderResourceView) _ShaderResourceView->Release();
}
