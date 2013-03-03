#include "MeshVertexShader.h"


MeshVertexShader::MeshVertexShader( char* szFileName, char* szFuncName)
	:VertexShader(szFileName, szFuncName)
	,_FileName(szFileName)
	,_FuncName(szFuncName)
{
}


MeshVertexShader::~MeshVertexShader(void)
{
	std::map<VertexShaderKey, VertexShaderRes*>::iterator it;
	for(it=_ShaderMap.begin();it!=_ShaderMap.end();it++)
	{
		delete it->second;
	}
}

void MeshVertexShader::SetShader(EMeshType MeshType, int NumTexcoord )
{
	VertexShaderKey Key;
	Key.MeshType = MeshType;
	Key.NumTexcoord = NumTexcoord;

	VertexShaderRes* pShaderRes = GetShaderRes(Key);
	pShaderRes->SetShader();
}

VertexShaderRes* MeshVertexShader::GetShaderRes( VertexShaderKey& Key )
{
	std::map<VertexShaderKey, VertexShaderRes*>::iterator it;
	it = _ShaderMap.find(Key);
	if (it != _ShaderMap.end())
	{
		return it->second;
	}
	else
	{
		VertexShaderRes* pShaderRes = new VertexShaderRes(_FileName.c_str(), _FuncName.c_str(), Key);
		_ShaderMap.insert(std::pair<VertexShaderKey, VertexShaderRes*>(Key, pShaderRes));
		return pShaderRes;
	}
}

VertexShaderRes::VertexShaderRes( const char* FileName, const char* FuncName, VertexShaderKey& SKey )
{
	std::vector<D3D10_SHADER_MACRO> Defines;
	if(SKey.NumTexcoord == 0)
	{
		D3D10_SHADER_MACRO Define = {"TEXCOORD", "0"};
		Defines.push_back(Define);
	}
	else if(SKey.NumTexcoord == 1)
	{
		D3D10_SHADER_MACRO Define = {"TEXCOORD", "1"};
		Defines.push_back(Define);
	}

	if(SKey.MeshType == GpuSkin)
	{
		D3D10_SHADER_MACRO Define = {"GPUSKINNING", "1"};
		Defines.push_back(Define);
	}
	else if(SKey.MeshType == Static)
	{
		D3D10_SHADER_MACRO Define = {"GPUSKINNING", "0"};
		Defines.push_back(Define);
	}

	D3D10_SHADER_MACRO Define;
	memset(&Define, 0, sizeof(D3D10_SHADER_MACRO));
	Defines.push_back(Define);

	int nLen = strlen(FileName)+1;

	wchar_t WFileName[1024];
	size_t RetSize;
	mbstowcs_s(&RetSize, WFileName, 1024, FileName, nLen);

	HRESULT hr;
	ID3DBlob* pVSBlob = NULL;
	hr = GEngine->CompileShaderFromFile( WFileName, &Defines.at(0), "VS", "vs_4_0", &pVSBlob );
	if( FAILED( hr ) )
	{
		MessageBox( NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK );
		assert(false);
	}

	// Create the vertex shader
	hr = GEngine->_Device->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &_VertexShader );
	if( FAILED( hr ) )
	{	

		pVSBlob->Release();
		assert(false);
	}

	if(SKey.NumTexcoord == 0)
	{
		if(SKey.MeshType == Static)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT numElements = ARRAYSIZE( layout );

			// Create the input layout
			hr = GEngine->_Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
				pVSBlob->GetBufferSize(), &_VertexLayout );
		}
		else if(SKey.MeshType == GpuSkin)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "WEIGHTS", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BONES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT numElements = ARRAYSIZE( layout );

			// Create the input layout
			hr = GEngine->_Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
				pVSBlob->GetBufferSize(), &_VertexLayout );
		}
	}
	else if(SKey.NumTexcoord == 1)
	{
		if(SKey.MeshType == Static)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT numElements = ARRAYSIZE( layout );

			// Create the input layout
			hr = GEngine->_Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
				pVSBlob->GetBufferSize(), &_VertexLayout );
		}
		else if(SKey.MeshType == GpuSkin)
		{
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "WEIGHTS", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BONES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			UINT numElements = ARRAYSIZE( layout );

			// Create the input layout
			hr = GEngine->_Device->CreateInputLayout( layout, numElements, pVSBlob->GetBufferPointer(),
				pVSBlob->GetBufferSize(), &_VertexLayout );
		}
	}


	pVSBlob->Release();
	if( FAILED( hr ) )
		assert(false);
}

VertexShaderRes::~VertexShaderRes()
{
	if(_VertexShader) _VertexShader->Release();
	if(_VertexLayout) _VertexLayout->Release();
}

void VertexShaderRes::SetShader()
{
	GEngine->_ImmediateContext->IASetInputLayout( _VertexLayout );
	GEngine->_ImmediateContext->VSSetShader( _VertexShader, NULL, 0 );
}
