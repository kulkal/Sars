#include <cassert>

#include "TextureDepth2D.h"
#include "Engine.h"

TextureDepth2D::TextureDepth2D(D3D11_TEXTURE2D_DESC& TexDesc, D3D11_DEPTH_STENCIL_VIEW_DESC& DSVDesc, D3D11_SHADER_RESOURCE_VIEW_DESC& SRVDesc)
	: Texture2D(TexDesc, SRVDesc, false)
	,_DepthStencilView(NULL)
	,_ReadOnlyDepthStencilView(NULL)
{
	HRESULT hr;
	hr = GEngine->_Device->CreateDepthStencilView( _Texture, &DSVDesc, &_DepthStencilView );
	if( FAILED( hr ) )
		assert(false);

	DSVDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH | D3D11_DSV_READ_ONLY_STENCIL;
	hr = GEngine->_Device->CreateDepthStencilView( _Texture, &DSVDesc, &_ReadOnlyDepthStencilView );
	if( FAILED( hr ) )
		assert(false);
}


TextureDepth2D::~TextureDepth2D(void)
{
	if(_DepthStencilView) _DepthStencilView->Release();
	if(_ReadOnlyDepthStencilView) _ReadOnlyDepthStencilView->Release();
}
