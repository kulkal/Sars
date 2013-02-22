#pragma once
#include <d3d11.h>
#include <xnamath.h>
#include <cassert>
#include "Engine.h"
#include "Util.h"


class Shader
{
protected:
	ID3D11Buffer*           _ConstantBuffer;
public:
	
	template<typename SCType>
	void CreateConstantBuffer()
	{
		HRESULT hr;
		D3D11_BUFFER_DESC bdc;
		ZeroMemory( &bdc, sizeof(bdc) );
		bdc.Usage = D3D11_USAGE_DEFAULT;
		bdc.ByteWidth = sizeof(SCType);
		bdc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bdc.CPUAccessFlags = 0;
		hr = GEngine->_Device->CreateBuffer( &bdc, NULL, &_ConstantBuffer );
		if( FAILED( hr ) )
			assert(false);

		SetD3DResourceDebugName("_DeferredSahdowtPSCB", _ConstantBuffer);
	}

	Shader(void);
	virtual ~Shader(void);
};

