#include "StaticMesh.h"
#include "Engine.h"
#include <cassert>

const int TRIANGLE_VERTEX_COUNT = 3;
const int VERTEX_STRIDE = 4;
const int NORMAL_STRIDE = 3;
const int UV_STRIDE = 2;



StaticMesh::StaticMesh(void)
	:
	_VertexBuffer(NULL),
	_IndexBuffer(NULL),
	_VertexStride(0),
	_PositionArray(NULL),
	_NormalArray(NULL),
	_TexCoordArray(NULL),
	_NumTexCoord(0),
	_NumTriangle(0),
	_NumVertex(0)
{
}


StaticMesh::~StaticMesh(void)
{
	if(_PositionArray) delete[] _PositionArray;
	if(_NormalArray) delete[] _NormalArray;
	if(_TexCoordArray) delete[] _TexCoordArray;
	if(_IndiceArray) delete[] _IndiceArray;

	if(_VertexBuffer) _VertexBuffer->Release();
	if(_IndexBuffer) _IndexBuffer->Release();
}

bool StaticMesh::ImportFromFbxMesh( FbxMesh* Mesh, FbxFileImporter* Importer )
{
	if (!Mesh->GetNode())
		return false;

	FbxNode* pNode = Mesh->GetNode();
	FbxAMatrix Geometry;
	FbxVector4 Translation, Rotation, Scaling;
	Translation = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	Rotation = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	Scaling = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	Geometry.SetT(Translation);
	Geometry.SetR(Rotation);
	Geometry.SetS(Scaling);

	//For Single Matrix situation, obtain transfrom matrix from eDESTINATION_SET, which include pivot offsets and pre/post rotations.
	FbxAMatrix& GlobalTransform = Importer->mScene->GetEvaluator()->GetNodeGlobalTransform(pNode);

	FbxAMatrix TotalMatrix;
	TotalMatrix = GlobalTransform * Geometry;

	FbxAMatrix TotalMatrixForNormal;
	TotalMatrixForNormal = TotalMatrix.Inverse();
	TotalMatrixForNormal = TotalMatrixForNormal.Transpose();

	if (!Mesh->IsTriangleMesh())
	{
		FbxGeometryConverter lConverter(Mesh->GetNode()->GetFbxManager());
		bool bSuccess;
		Mesh = lConverter.TriangulateMeshAdvance(Mesh, bSuccess);
	}
	
	const int PolygonCount = Mesh->GetPolygonCount();

	FbxLayerElementArrayTemplate<int>* MaterialIndice = NULL;
	FbxGeometryElement::EMappingMode MaterialMappingMode = FbxGeometryElement::eNone;


	FbxGeometryElementMaterial * ElementMaterial = Mesh->GetElementMaterial();
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
					if ((int)_SubMeshArray.size() < lMaterialIndex + 1)
					{
						_SubMeshArray.resize(lMaterialIndex + 1);
					}
					if (_SubMeshArray[lMaterialIndex] == NULL)
					{
						_SubMeshArray[lMaterialIndex] = new SubMesh;
					}
					_SubMeshArray[lMaterialIndex]->_TriangleCount += 1;
				}

				// Make sure we have no "holes" (NULL) in the mSubMeshes table. This can happen
				// if, in the loop above, we resized the mSubMeshes by more than one slot.
				for (int i = 0; i < (int)_SubMeshArray.size(); i++)
				{
					if (_SubMeshArray[i] == NULL)
						_SubMeshArray[i] = new SubMesh;

				}

				// Record the offset (how many vertex)
				const int lMaterialCount = _SubMeshArray.size();
				int lOffset = 0;
				for (int lIndex = 0; lIndex < lMaterialCount; ++lIndex)
				{
					_SubMeshArray[lIndex]->_IndexOffset = lOffset;
					lOffset += _SubMeshArray[lIndex]->_TriangleCount * 3;
					// This will be used as counter in the following procedures, reset to zero
					_SubMeshArray[lIndex]->_TriangleCount = 0;
				}
				FBX_ASSERT(lOffset == PolygonCount * 3);
			}
		}

		
	}

	// All faces will use the same material.
	if (_SubMeshArray.size() == 0)
	{
		_SubMeshArray.resize(1);
		_SubMeshArray[0] = new SubMesh();
	}

	// Congregate all the data of a mesh to be cached in VBOs.
	// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
	bool mHasNormal = Mesh->GetElementNormalCount() > 0;
	bool mHasUV = Mesh->GetElementUVCount() > 0;
	bool mAllByControlPoint = true;
	FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
	FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
	if (mHasNormal)
	{
		lNormalMappingMode = Mesh->GetElementNormal(0)->GetMappingMode();
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
		lUVMappingMode = Mesh->GetElementUV(0)->GetMappingMode();
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
	int lPolygonVertexCount = Mesh->GetControlPointsCount();
	if (!mAllByControlPoint)
	{
		lPolygonVertexCount = PolygonCount * TRIANGLE_VERTEX_COUNT;
	}
	_PositionArray = new XMFLOAT3[lPolygonVertexCount];
	_IndiceArray = new WORD[PolygonCount * TRIANGLE_VERTEX_COUNT];

	//float * lVertices = new float[lPolygonVertexCount * VERTEX_STRIDE];
	//unsigned int * lIndices = new unsigned int[PolygonCount * TRIANGLE_VERTEX_COUNT];
	//float * lNormals = NULL;
	if (mHasNormal)
	{
		//lNormals = new float[lPolygonVertexCount * NORMAL_STRIDE];
		_NormalArray = new XMFLOAT3[lPolygonVertexCount];
	}
	//float * lUVs = NULL;
	FbxStringList lUVNames;
	Mesh->GetUVSetNames(lUVNames);
	const char * lUVName = NULL;
	if (mHasUV && lUVNames.GetCount())
	{
		//lUVs = new float[lPolygonVertexCount * UV_STRIDE];
		_TexCoordArray = new XMFLOAT2[lPolygonVertexCount];
		lUVName = lUVNames[0];
	}

	// Populate the array with vertex attribute, if by control point.
	const FbxVector4 * lControlPoints = Mesh->GetControlPoints();
	FbxVector4 lCurrentVertex;
	FbxVector4 lCurrentNormal;
	FbxVector2 lCurrentUV;
	if (mAllByControlPoint)
	{
		const FbxGeometryElementNormal * lNormalElement = NULL;
		const FbxGeometryElementUV * lUVElement = NULL;
		if (mHasNormal)
		{
			lNormalElement = Mesh->GetElementNormal(0);
		}
		if (mHasUV)
		{
			lUVElement = Mesh->GetElementUV(0);
		}
		for (int lIndex = 0; lIndex < lPolygonVertexCount; ++lIndex)
		{
			// Save the vertex position.
			lCurrentVertex = lControlPoints[lIndex];
			//lVertices[lIndex * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
			//lVertices[lIndex * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
			//lVertices[lIndex * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
			//lVertices[lIndex * VERTEX_STRIDE + 3] = 1;
			FbxVector4 FinalPosition = TotalMatrix.MultT(lCurrentVertex);


			_PositionArray[lIndex].x = static_cast<float>(FinalPosition[0]);
			_PositionArray[lIndex].y = static_cast<float>(FinalPosition[1]);
			_PositionArray[lIndex].z = static_cast<float>(FinalPosition[2]);

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

				FbxVector4 FinalNormal = TotalMatrixForNormal.MultT(lCurrentNormal);

				_NormalArray[lIndex].x = static_cast<float>(FinalNormal[0]);
				_NormalArray[lIndex].y = static_cast<float>(FinalNormal[1]);
				_NormalArray[lIndex].z = static_cast<float>(lCurrentNormal[2]);
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
				_TexCoordArray[lIndex].x = static_cast<float>(lCurrentUV[0]);
				_TexCoordArray[lIndex].y = static_cast<float>(lCurrentUV[1]);
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
		const int lIndexOffset = _SubMeshArray[lMaterialIndex]->_IndexOffset +
			_SubMeshArray[lMaterialIndex]->_TriangleCount * 3;
		for (int lVerticeIndex = 0; lVerticeIndex < TRIANGLE_VERTEX_COUNT; ++lVerticeIndex)
		{
			const int lControlPointIndex = Mesh->GetPolygonVertex(lPolygonIndex, lVerticeIndex);

			if (mAllByControlPoint)
			{
				//lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lControlPointIndex);
				_IndiceArray[lIndexOffset + lVerticeIndex] = static_cast<WORD>(lControlPointIndex);

			/*	sprintf(StrLog, "%d, \n", IndiceArray[lIndexOffset + lVerticeIndex]);
				OutputDebugStringA(StrLog);*/
			}
			// Populate the array with vertex attribute, if by polygon vertex.
			else
			{
				//lIndices[lIndexOffset + lVerticeIndex] = static_cast<unsigned int>(lVertexCount);
				_IndiceArray[lIndexOffset + lVerticeIndex] = static_cast<WORD>(lVertexCount);


				lCurrentVertex = lControlPoints[lControlPointIndex];
				///lVertices[lVertexCount * VERTEX_STRIDE] = static_cast<float>(lCurrentVertex[0]);
				//lVertices[lVertexCount * VERTEX_STRIDE + 1] = static_cast<float>(lCurrentVertex[1]);
				//lVertices[lVertexCount * VERTEX_STRIDE + 2] = static_cast<float>(lCurrentVertex[2]);
				//lVertices[lVertexCount * VERTEX_STRIDE + 3] = 1;
				//lCurrentVertex[0] = -lCurrentVertex[0];
				FbxVector4 FinalPosition = TotalMatrix.MultT(lCurrentVertex);

				_PositionArray[lVertexCount].x =  static_cast<float>(FinalPosition[0]);
				_PositionArray[lVertexCount].y =  static_cast<float>(FinalPosition[1]);
				_PositionArray[lVertexCount].z =  static_cast<float>(FinalPosition[2]);



				if (mHasNormal)
				{
					Mesh->GetPolygonVertexNormal(lPolygonIndex, lVerticeIndex, lCurrentNormal);
					//lNormals[lVertexCount * NORMAL_STRIDE] = static_cast<float>(lCurrentNormal[0]);
					//lNormals[lVertexCount * NORMAL_STRIDE + 1] = static_cast<float>(lCurrentNormal[1]);
					//lNormals[lVertexCount * NORMAL_STRIDE + 2] = static_cast<float>(lCurrentNormal[2]);

					FbxVector4 FinalNormal = TotalMatrixForNormal.MultT(lCurrentNormal);

					_NormalArray[lVertexCount].x = static_cast<float>(FinalNormal[0]);
					_NormalArray[lVertexCount].y = static_cast<float>(FinalNormal[1]);
					_NormalArray[lVertexCount].z = static_cast<float>(FinalNormal[2]);

				}

				if (mHasUV)
				{
					Mesh->GetPolygonVertexUV(lPolygonIndex, lVerticeIndex, lUVName, lCurrentUV);
					//lUVs[lVertexCount * UV_STRIDE] = static_cast<float>(lCurrentUV[0]);
					//lUVs[lVertexCount * UV_STRIDE + 1] = static_cast<float>(lCurrentUV[1]);

					_TexCoordArray[lVertexCount].x = static_cast<float>(lCurrentUV[0]);
					_TexCoordArray[lVertexCount].y = static_cast<float>(lCurrentUV[1]);
				}
			}
			++lVertexCount;
		}
		_SubMeshArray[lMaterialIndex]->_TriangleCount += 1;
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
			Vertices[i].Position = _PositionArray[i];
			Vertices[i].Normal = _NormalArray[i];
		}
		InitData.pSysMem = Vertices;
		hr = GEngine->_Device->CreateBuffer( &bd, &InitData, &_VertexBuffer );
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
			Vertices[i].Position = _PositionArray[i];
			Vertices[i].Normal = _NormalArray[i];
			Vertices[i].TexCoord = _TexCoordArray[i];
		}
		InitData.pSysMem = Vertices;
		hr = GEngine->_Device->CreateBuffer( &bd, &InitData, &_VertexBuffer );
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
	InitData.pSysMem = _IndiceArray;
	hr = GEngine->_Device->CreateBuffer( &bd, &InitData, &_IndexBuffer );
	if( FAILED( hr ) )
	{
		assert(false);
		return false;
	}

	_NumTriangle = PolygonCount;
	_NumVertex = lPolygonVertexCount;

	if(_NormalArray != NULL && _TexCoordArray == NULL)
	{
		_VertexStride = sizeof(NormalVertex);
		_NumTexCoord = 0;
	}
	else if(_NormalArray != NULL && _TexCoordArray != NULL)
	{
		_VertexStride = sizeof(NormalTexVertex);
		_NumTexCoord = 1;
	}

	return true;
}
