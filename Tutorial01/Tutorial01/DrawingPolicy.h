#pragma once

#include <d3d11.h>
#include <d3dx11.h>
#include <xnamath.h>
#include <map>
#include <cassert>

#include "Engine.h"
#include "StaticMesh.h"
#include "SkeletalMesh.h"
#include "SkeletalMeshRenderData.h"
#include "ShaderRes.h"

class StaticMesh;
class SkeletalMesh;
class SkeletalMeshRenderData;
class DrawingPolicy
{
protected:
	std::map<ShaderMapKey, ShaderRes*> ShaderMap;
	std::string FileName;

public:
	virtual void DrawStaticMesh(StaticMesh* pMesh) = 0;
	virtual void DrawSkeletalMesh(SkeletalMesh* pMesh) = 0;
	virtual void DrawSkeletalMeshData(SkeletalMeshRenderData* pRenderData) = 0;

	ShaderRes* GetShaderRes(int NumTex, EVertexProcessingType VPType);

	DrawingPolicy(void);
	virtual ~DrawingPolicy(void);
};

