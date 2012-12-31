#include "StaticMesh.h"
#include "Engine.h"
#include <cassert>

const int TRIANGLE_VERTEX_COUNT = 3;
const int VERTEX_STRIDE = 4;
const int NORMAL_STRIDE = 3;
const int UV_STRIDE = 2;



StaticMesh::StaticMesh(void)
	:
	VertexBuffer(NULL),
	IndexBuffer(NULL)
{
}


StaticMesh::~StaticMesh(void)
{
	if(PositionArray) delete[] PositionArray;
	if(NormalArray) delete[] NormalArray;
	if(TexCoordArray) delete[] TexCoordArray;
	if(IndiceArray) delete[] IndiceArray;

	if(VertexBuffer) VertexBuffer->Release();
	if(IndexBuffer) IndexBuffer->Release();
}

bool StaticMesh::ImportFromFbxMesh( FbxMesh* pMesh )
{
	char StrLog[1024];
	if (!pMesh->GetNode())
		return false;

	if (!pMesh->IsTriangleMesh())
	{
		FbxGeometryConverter lConverter(pMesh->GetNode()->GetFbxManager());
		bool bSuccess;
		pMesh = lConverter.TriangulateMeshAdvance(pMesh, bSuccess);
	}

	const int PolygonCount = pMesh->GetPolygonCount();

	FbxLayerElementArrayTemplate<int>* MaterialIndice = NULL;
	FbxGeometryElement::EMappingMode MaterialMappingMode = FbxGeometryElement::eNone;


	FbxGeometryElementMaterial * ElementMaterial = pMesh->GetElementMaterial();
	if (ElementMaterial)
	{
		MaterialIndice = &ElementMaterial->GetIndexArray();
		MaterialMappingMode = ElementMaterial->GetMappingMode();
		if (MaterialIndice && MaterialMappingMode == FbxGeometryElement::eByPolygon)
		{
			FBX_ASSERT(MaterialIndice->GetCount() == PolygonCount);
			if (MaterialIndice->GetCount() == PolygonCount)
			{
				// Count the faces of each material
				for (int PolygonIndex = 0; PolygonIndex < PolygonCount; ++PolygonIndex)
				{
					const int lMaterialIndex = MaterialIndice->GetAt(PolygonIndex);
					if ((int)SubMeshArray.size() < lMaterialIndex + 1)
					{
						SubMeshArray.resize(lMaterialIndex + 1);
					}
					if (SubMeshArray[lMaterialIndex] == NULL)
					{
						SubMeshArray[lMaterialIndex] = new SubMesh;
					}
					SubMeshArray[lMaterialIndex]->TriangleCount += 1;
				}

				// Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
				// if, in the loop above, we resized the mSubMeshes by more than one slot.
				for (int i = 0; i < (int)SubMeshArray.size(); i++)
				{
					if (SubMeshArray[i] == NULL)
						SubMeshArray[i] = new SubMesh;

				}

				// Record the offset (how many vertex)
				const int lMaterialCount = SubMeshArray.size();
				int lOffset = 0;
				for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
				{
					SubMeshArray[lIndex]->IndexOffset = lOffset;
					lOffset += SubMeshArray[lIndex]->TriangleCount * 3;
					// This will be used as counter in the following procedures, reset to zero
					SubMeshArray[lIndex]->TriangleCount = 0;
				}
				FBX_ASSERT(lOffset == PolygonCount * 3);
			}
		}

		
	}

	// All faces will use the same material.
	if (SubMeshArray.size() == 0)
	{
		SubMeshArray.resize(1);
		SubMeshArray[0] = new SubMesh();
	}

	// Congregate all the data of a mesh to be cached in VBOs.
	// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
	bool mHasNormal = pMesh->GetElementNormalCount() > 0;
	bool mHasUV = pMesh->GetElementUVCount() > 0;
	bool mAllByControlPoint = true;
	FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
	if (mHasNormal)
	{
		lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
		if (lNormalMappingMode == FbxGeometryElement::eNone)
		{
			mHasNormal = false;
		}
		if (mHasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
		{
			mAllByControlPoint = false;
		}
	}
	if (mHasUV)
	{
		lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
		if (lUVMappingMode == FbxGeometryElement::eNone)
		{
			mHasUV = false;
		}
		if (mHasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
		{
			mAllByControlPoint = false;
		}
	}


	// Allocate the array memory, by control point or by polygon vertex.
	int lPolygonVertexCount = pMesh->GetControlPointsCount();
	if (!mAllByControlPoint)
	{
		lPolygonVertexCount = PolygonCount * TRIANGLE_VERTEX_COUNT;
	}
	PositionArray = new XMFLOAT3[lPolygonVertexCount];
	IndiceArray = new WORD[PolygonCount * TRIANGLE_VERTEX_COUNT];

	//float * lVertices = new float[lPolygonVertexCount * VERTEX_STRIDE];
	//unsigned int * lIndices = new unsigned int[PolygonCount * TRIANGLE_VERTEX_COUNT];
	//float * lNormals = NULL;
	if (mHasNormal)
	{
		//lNormals = new float[lPolygonVertexCount * NORMAL_STRIDE];
		NormalArray = new XMFLOAT3[lPolygonVertexCount];
	}
	//float * lUVs = NULL;
	FbxStringList lUVNames;
	pMesh->GetUVSetNames(lUVNames);
	const char * lUVName = NULL;
	if (mHasUV && lUVNames.GetCount())
	{
		//lUVs = new float[lPolygonVertexCount * UV_STRIDE];
		TexCoordArray = new XMFLOAT2[lPolygonVertexCount];
		lUVName = lUVNames[0];
	}

	// Populate the array with vertex attribute, if by control point.
	const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
	FbxVector4 lCurrentVertex;
	FbxVector4 lCurrentNormal;
	FbxVector2 lCurrentUV;
	if (mAllByControlPoint)
	{
		const FbxGeometryElementNormal * lNormalElement = NULL;
		const FbxGeometryElementUV * lUVElement = NULL;
		if (mHasNormal)
		{
			lNormalElement = pMesh->GetElementNormal(0);
		}
		if (mHasUV)
		{
			lUVElement = pMesh->GetElementUV(0);
		}
		for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
		{
			// Save the vertex position.
			lCurrentVertex = lControlPoints[lIndex];
			//lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
			//lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
			//lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
			//lVertices[lIndex * VERTEX_STRIDE + 3] = 1;
			PositionArray[lIndex].x = static_cast<float>(lCurrentVertex[0]);
			PositionArray[lIndex].y = static_cast<float>(lCurrentVertex[1]);
			PositionArray[lIndex].z = static_cast<float>(lCurrentVertex[2]);

			/*sprintf(StrLog, "Vertex %d %d\n", lIndex);
			sprintf(StrLog, "x : %d, y : %d, z : %d\n", PositionArray[lIndex].x, PositionArray[lIndex].y, PositionArray[lIndex].z);
			OutputDebugStringA(StrLog);
*/
			// Save the normal.
			if (mHasNormal)
			{
				int lNormalIndex = lIndex;
				if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lNormalIndex = lNormalElement->GetIndexArray().GetAt(lIndex);
				}
				lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);
				//lNormals[lIndex * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
				//lNormals[lIndex * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
				//lNormals[lIndex * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);
				NormalArray[lIndex].x = static_cast<float>(lCurrentNormal[0]);
				NormalArray[lIndex].y = static_cast<float>(lCurrentNormal[1]);
				NormalArray[lIndex].z = static_cast<float>(lCurrentNormal[2]);
			}

			// Save the UV.
			if (mHasUV)
			{
				int lUVIndex = lIndex;
				if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
				{
					lUVIndex = lUVElement->GetIndexArray().GetAt(lIndex);
				}
				lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);
				//lUVs[lIndex * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
				//lUVs[lIndex * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);
				TexCoordArray[lIndex].x = static_cast<float>(lCurrentUV[0]);
				TexCoordArray[lIndex].y = static_cast<float>(lCurrentUV[1]);
			}
		}

	}

	int lVertexCount = 0;
	for (int lPolygonIndex = 0; lPolygonIndex < PolygonCount; ++lPolygonIndex)
	{
	/*	sprintf(StrLog, "\npolygon %d\n", lPolygonIndex);
		OutputDebugStringA(StrLog);
*/
		// The material for current face.
		int lMaterialIndex = 0;
		if (MaterialIndice && MaterialMappingMode == FbxGeometryElement::eByPolygon)
		{
			lMaterialIndex = MaterialIndice->GetAt(lPolygonIndex);
		}

		// Where should I save the vertex attribute index, according to the material
		const int lIndexOffset = SubMeshArray[lMaterialIndex]->IndexOffset +
			SubMeshArray[lMaterialIndex]->TriangleCount * 3;
		for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
		{
			const int lControlPointIndex = pMesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

			if (mAllByControlPoint)
			{
				//lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
				IndiceArray[lIndexOffset + lVerticeIndex] = static_cast<WORD>(lControlPointIndex);

			/*	sprintf(StrLog, "%d, \n", IndiceArray[lIndexOffset + lVerticeIndex]);
				OutputDebugStringA(StrLog);*/
			}
			// Populate the array with vertex attribute, if by polygon vertex.
			else
			{
				//lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);
				IndiceArray[lIndexOffset + lVerticeIndex] = static_cast<WORD>(lVertexCount);


				lCurrentVertex = lControlPoints[lControlPointIndex];
				///lVertices[lVertexCount * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
				//lVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
				//lVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
				//lVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;

				PositionArray[lVertexCount].x =  static_cast<float>(lCurrentVertex[0]);
				PositionArray[lVertexCount].y =  static_cast<float>(lCurrentVertex[1]);
				PositionArray[lVertexCount].z =  static_cast<float>(lCurrentVertex[2]);


				if (mHasNormal)
				{
					pMesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
					//lNormals[lVertexCount * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
					//lNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
					//lNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);

					NormalArray[lVertexCount].x = static_cast<float>(lCurrentNormal[0]);
					NormalArray[lVertexCount].y = static_cast<float>(lCurrentNormal[1]);
					NormalArray[lVertexCount].z = static_cast<float>(lCurrentNormal[2]);

				}

				if (mHasUV)
				{
					pMesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV);
					//lUVs[lVertexCount * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
					//lUVs[lVertexCount * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);

					TexCoordArray[lVertexCount].x = static_cast<float>(lCurrentUV[0]);
					TexCoordArray[lVertexCount].y = static_cast<float>(lCurrentUV[1]);
				}
			}
			++lVertexCount;
		}
		SubMeshArray[lMaterialIndex]->TriangleCount += 1;
	}

	


	HRESULT hr;
	D3D11_BUFFER_DESC bd;
	ZeroMemory( &bd, sizeof(bd) );
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory( &InitData, sizeof(InitData) );
	if(mHasNormal == true && mHasUV == false)
	{
		bd.ByteWidth = sizeof( NormalVertex ) * lPolygonVertexCount;
		NormalVertex* Vertices = new NormalVertex[lPolygonVertexCount];
		for(int i = 0;i<lPolygonVertexCount;i++)
		{
			Vertices[i].Position = PositionArray[i];
			Vertices[i].Normal = NormalArray[i];
		}
		InitData.pSysMem = Vertices;
		hr = GEngine->Device->CreateBuffer( &bd, &InitData, &VertexBuffer );
		if( FAILED( hr ) )
		{
			assert(false);
			return false;
		}
		delete[] Vertices;
	}
	else if(mHasNormal == true && mHasUV == true)
	{
		bd.ByteWidth = sizeof( NormalTexVertex ) * lPolygonVertexCount;
		NormalTexVertex* Vertices = new NormalTexVertex[lPolygonVertexCount];
		for(int i = 0;i<lPolygonVertexCount;i++)
		{
			Vertices[i].Position = PositionArray[i];
			Vertices[i].Normal = NormalArray[i];
		}
		InitData.pSysMem = Vertices;
		hr = GEngine->Device->CreateBuffer( &bd, &InitData, &VertexBuffer );
		if( FAILED( hr ) )
		{
			assert(false);
			return false;
		}
		delete[] Vertices;
	}
	

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof( WORD ) * PolygonCount * TRIANGLE_VERTEX_COUNT;        // 36 vertices needed for 12 triangles in a triangle list
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = IndiceArray;
	hr = GEngine->Device->CreateBuffer( &bd, &InitData, &IndexBuffer );
	if( FAILED( hr ) )
	{
		assert(false);
		return false;
	}

	NumTriangle = PolygonCount;
	NumVertex = lPolygonVertexCount;

	return true;
}
