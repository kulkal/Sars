#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include "map"

#include "ShaderRes.h"
class StaticMesh;

class DrawingPolicy
{
protected:
	std::map<ShaderMapKey, ShaderRes*> ShaderMap;
	std::string FileName;

public:
	virtual void DrawStaticMesh(StaticMesh* pMesh) = 0;

	ShaderRes* GetShaderRes(ENumTexCoord NumTex);

	DrawingPolicy(void);
	virtual ~DrawingPolicy(void);
};

