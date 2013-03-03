#pragma once
#include "vertexshader.h"
#include <map>

enum EMeshType
{
	Static,
	GpuSkin
};



struct VertexShaderKey
{
	EMeshType MeshType;
	int NumTexcoord;
	bool operator<(const VertexShaderKey& other) const        
	{             
		if( NumTexcoord < other.NumTexcoord ) return true;
		if( NumTexcoord > other.NumTexcoord ) return false;

		if( MeshType < other.MeshType ) return true;
		if( MeshType > other.MeshType ) return false;

		return false;
	};
};

class VertexShaderRes
{
	ID3D11InputLayout*      _VertexLayout;
	ID3D11VertexShader*     _VertexShader;
public:
	void SetShader();
	VertexShaderRes(const char* FileName, const char* FuncName, VertexShaderKey& SKey);
	~VertexShaderRes();
};

class MeshVertexShader :
	public VertexShader
{
	std::string _FileName;
	std::string _FuncName;
	std::map<VertexShaderKey, VertexShaderRes*> _ShaderMap;
public:
	VertexShaderRes* GetShaderRes(VertexShaderKey& Key);
	void SetShader(EMeshType MeshType, int NumTexcoord);
	void SetShaderParameter();

	MeshVertexShader( char* szFileName, char* szFuncName);
	virtual ~MeshVertexShader(void);
};

