#include "VertexShader.h"
#include <cassert>


VertexShader::VertexShader(char* szFileName, char* szFuncName)
	:_VertexShader(NULL)
	,_VertexLayout(NULL)
{

}

VertexShader::VertexShader(char* szFileName, char* szFuncName, const D3D11_INPUT_ELEMENT_DESC* layout, int numLayout, D3D10_SHADER_MACRO* pDefines)
	:_VertexShader(NULL)
	,_VertexLayout(NULL)
{
	HRESULT hr;
	int nLen = strlen(szFileName)+1;

	wchar_t WFileName[1024];
	size_t RetSize;
	mbstowcs_s(&RetSize, WFileName, 1024, szFileName, nLen);

	ID3DBlob* pBlob = NULL;
	GEngine->CompileShaderFromFile( WFileName, NULL, szFuncName, "vs_4_0", &pBlob ) ;
	hr = GEngine->_Device->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &_VertexShader ) ;
	if( FAILED( hr ) )
		assert(false);

	hr = GEngine->_Device->CreateInputLayout( layout, numLayout, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &_VertexLayout ) ;
	if( FAILED( hr ) )
		assert(false);

	if(pBlob) pBlob->Release();
}


VertexShader::~VertexShader(void)
{
	if(_VertexShader) _VertexShader->Release();
	if(_VertexLayout) _VertexLayout->Release();
}

void VertexShader::SetShader()
{
	GEngine->_ImmediateContext->IASetInputLayout( _VertexLayout );
	GEngine->_ImmediateContext->VSSetShader( _VertexShader, NULL, 0 );
}
