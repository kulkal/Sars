#include "DrawingPolicy.h"


DrawingPolicy::DrawingPolicy(void)
	:FileName("")

{
}


DrawingPolicy::~DrawingPolicy(void)
{
	std::map<ShaderMapKey, ShaderRes*>::iterator it;
	for(it=ShaderMap.begin();it!=ShaderMap.end();it++)
	{
		delete it->second;
	}
}


ShaderRes* DrawingPolicy::GetShaderRes( ENumTexCoord NumTex, EVertexProcessingType VPType)
{
	ShaderMapKey SKey;
	SKey.NumTex = NumTex;
	SKey.VertexProcessingType = VPType;
	std::map<ShaderMapKey, ShaderRes*>::iterator it;
	it = ShaderMap.find(SKey);
	if (it != ShaderMap.end())
	{
		return it->second;
	}
	else
	{
		ShaderRes* pShaderRes = new ShaderRes;
		pShaderRes->CreateShader(FileName.c_str(), SKey);
		ShaderMap.insert(std::pair<ShaderMapKey, ShaderRes*>(SKey, pShaderRes));
		return pShaderRes;
	}
}