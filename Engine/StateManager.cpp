#include <cassert>
#include "StateManager.h"
#include "Engine.h"

StateManager* GStateManager = NULL;


StateManager::StateManager(void)
{
}


StateManager::~StateManager(void)
{
	for(UINT i=0;i<_BlendStateArray.size();i++)
	{
		BlendStateData& BS = _BlendStateArray[i];
		BS.BS->Release();
	}

	for(UINT i=0;i<_DepthStencilStateArray.size();i++)
	{
		DepthStencilStateData& DSS = _DepthStencilStateArray[i];
		DSS.DSS->Release();
	}

	for(UINT i=0;i<_SamplerStateArray.size();i++)
	{
		SamplerStateData& SS = _SamplerStateArray[i];
		SS.SS->Release();
	}

	for(UINT i=0;i<_RasterStateArra.size();i++)
	{
		_RasterStateArra[i]->Release();
	}
}

void StateManager::Init()
{
	InitBlendStates();
	InitDepthStencilStates();
	InitSamplerStates();
	InitRasterStates();
}

void StateManager::SetBlendState(EBlendState eBS)
{
	GEngine->_ImmediateContext->OMSetBlendState(_BlendStateArray[eBS].BS, _BlendStateArray[eBS].BlendFactor, _BlendStateArray[eBS].SampleMask);

}

void StateManager::SetDepthStencilState( EDepthStencilState eDSS )
{
	GEngine->_ImmediateContext->OMSetDepthStencilState(_DepthStencilStateArray[eDSS].DSS, _DepthStencilStateArray[eDSS].StencilRef);
}

void StateManager::SetPSSampler( int StartSlot, ESamplerState eSS )
{
	GEngine->_ImmediateContext->PSSetSamplers( StartSlot, 1, &_SamplerStateArray[eSS].SS);
}

void StateManager::InitBlendStates()
{
	// blend state
	_BlendStateArray.resize(SIZE_BLENDSTATE);

	CD3D11_BLEND_DESC DescBlend(D3D11_DEFAULT);

	GEngine->_Device->CreateBlendState(&DescBlend, &_BlendStateArray[BS_NORMAL].BS);

	DescBlend.RenderTarget[0].BlendEnable = true;
	DescBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	DescBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	GEngine->_Device->CreateBlendState(&DescBlend, &_BlendStateArray[BS_LIGHTING].BS);

	/*DescBlend.RenderTarget[0].BlendEnable = true;
	DescBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT   ;
	DescBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	DescBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_REV_SUBTRACT   ;
*/
	DescBlend.RenderTarget[0].BlendEnable = true;
	DescBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;//D3D11_BLEND_DEST_COLOR ;
	DescBlend.RenderTarget[0].DestBlend =   D3D11_BLEND_SRC_COLOR;
	DescBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD   ;
	DescBlend.RenderTarget[0].SrcBlendAlpha =  D3D11_BLEND_DEST_ALPHA;
	DescBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_SRC_ALPHA ;
	DescBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD   ;
	GEngine->_Device->CreateBlendState(&DescBlend, &_BlendStateArray[BS_SHADOW].BS);
}

void StateManager::InitDepthStencilStates()
{
	// depth stencil state
	_DepthStencilStateArray.resize(SIZE_DEPTHSTENCILSTATE);

	// DS_GBUFFER_PASS
	D3D11_DEPTH_STENCIL_DESC  DSStateDesc;
	ZeroMemory( &DSStateDesc, sizeof(DSStateDesc) );
	DSStateDesc.DepthEnable = TRUE;
	DSStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	DSStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSStateDesc.StencilEnable = FALSE;
	DSStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DSStateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
	{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	DSStateDesc.FrontFace = defaultStencilOp;
	DSStateDesc.BackFace = defaultStencilOp;

	_DepthStencilStateArray[DS_GBUFFER_PASS].StencilRef = 0;
	GEngine->_Device->CreateDepthStencilState(&DSStateDesc, &_DepthStencilStateArray[DS_GBUFFER_PASS].DSS);

	//DS_LIGHTING_PASS
	DSStateDesc.DepthEnable = FALSE;
	DSStateDesc.StencilEnable = FALSE;
	DSStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	GEngine->_Device->CreateDepthStencilState(&DSStateDesc, &_DepthStencilStateArray[DS_LIGHTING_PASS].DSS);
}

void StateManager::InitSamplerStates()
{
	HRESULT hr;
	_SamplerStateArray.resize(SIZE_SAMPLERSTATE);

	// point
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = GEngine->_Device->CreateSamplerState( &sampDesc, &_SamplerStateArray[SS_POINT].SS );
	if( FAILED( hr ) )
		assert(false);

	// linear
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = GEngine->_Device->CreateSamplerState( &sampDesc, &_SamplerStateArray[SS_LINEAR].SS );
	if( FAILED( hr ) )
		assert(false);

	// shadow
	ZeroMemory( &sampDesc, sizeof(sampDesc) );
	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[0] = 1.f;
	sampDesc.BorderColor[1] = 1.f;
	sampDesc.BorderColor[2] = 1.f;
	sampDesc.BorderColor[3] = 1.f;
	hr = GEngine->_Device->CreateSamplerState( &sampDesc, &_SamplerStateArray[SS_SHADOW].SS );
	if( FAILED( hr ) )
		assert(false);
}

void StateManager::InitRasterStates()
{
	HRESULT hr;
	
	_RasterStateArra.resize(SIZE_RASTERSTATE);

	D3D11_RASTERIZER_DESC drd =
	{
		D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
		D3D11_CULL_FRONT,//D3D11_CULL_MODE CullMode;
		FALSE, //BOOL FrontCounterClockwise;
		0, //INT DepthBias;
		0.0f,//FLOAT DepthBiasClamp;
		0.0f,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		FALSE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;        
	};

	hr = GEngine->_Device->CreateRasterizerState(&drd, &_RasterStateArra[RS_NORMAL]);
	if ( FAILED( hr ) )
		assert(false);

	D3D11_RASTERIZER_DESC drdShadow =
	{
		D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
		D3D11_CULL_FRONT,//D3D11_CULL_MODE CullMode;
		FALSE, //BOOL FrontCounterClockwise;
		0, //INT DepthBias;
		0.0f,//FLOAT DepthBiasClamp;
		1.f,//FLOAT SlopeScaledDepthBias;
		TRUE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		FALSE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;        
	};
	hr = GEngine->_Device->CreateRasterizerState(&drdShadow, &_RasterStateArra[RS_SHADOWMAP]);
	if ( FAILED( hr ) )
		assert(false);
}

void StateManager::SetRasterizerState( ERasterState eRS )
{
	GEngine->_ImmediateContext->RSSetState(_RasterStateArra[eRS]);
}
