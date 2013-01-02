#include "SkeletalMesh.h"
#include "Engine.h"
#include <cassert>

const int TRIANGLE_VERTEX_COUNT = 3;
const int VERTEX_STRIDE = 4;
const int NORMAL_STRIDE = 3;
const int UV_STRIDE = 2;
void MatrixScale(FbxAMatrix& pMatrix, double pValue)
{
	int i,j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pMatrix[i][j] *= pValue;
		}
	}
}


// Add a value to all the elements in the diagonal of the matrix.
void MatrixAddToDiagonal(FbxAMatrix& pMatrix, double pValue)
{
	pMatrix[0][0] += pValue;
	pMatrix[1][1] += pValue;
	pMatrix[2][2] += pValue;
	pMatrix[3][3] += pValue;
}


// Sum two matrices element by element.
void MatrixAdd(FbxAMatrix& pDstMatrix, FbxAMatrix& pSrcMatrix)
{
	int i,j;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			pDstMatrix[i][j] += pSrcMatrix[i][j];
		}
	}
}

void ComputeLinearDeformation(FbxAMatrix& pGlobalPosition, 
	FbxMesh* pMesh, 
	FbxTime& pTime, 
	FbxVector4* pVertexArray,
	FbxPose* pPose)
{
	// All the links must have the same link mode.
	FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)pMesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

	int lVertexCount = pMesh->GetControlPointsCount();
	FbxAMatrix* lClusterDeformation = new FbxAMatrix[lVertexCount];
	memset(lClusterDeformation, 0, lVertexCount * sizeof(FbxAMatrix));

	double* lClusterWeight = new double[lVertexCount];
	memset(lClusterWeight, 0, lVertexCount * sizeof(double));

	if (lClusterMode == FbxCluster::eAdditive)
	{
		for (int i = 0; i < lVertexCount; ++i)
		{
			lClusterDeformation[i].SetIdentity();
		}
	}

	// For all skins and all clusters, accumulate their deformation and weight
	// on each vertices and store them in lClusterDeformation and lClusterWeight.
	int lSkinCount = pMesh->GetDeformerCount(FbxDeformer::eSkin);
	for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
	{
		FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);

		int lClusterCount = lSkinDeformer->GetClusterCount();
		for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
		{
			FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
			if (!lCluster->GetLink())
				continue;

			FbxAMatrix lVertexTransformMatrix;
			//ComputeClusterDeformation(pGlobalPosition, pMesh, lCluster, lVertexTransformMatrix, pTime, pPose);

			int lVertexIndexCount = lCluster->GetControlPointIndicesCount();
			for (int k = 0; k < lVertexIndexCount; ++k) 
			{            
				int lIndex = lCluster->GetControlPointIndices()[k];

				// Sometimes, the mesh can have less points than at the time of the skinning
				// because a smooth operator was active when skinning but has been deactivated during export.
				if (lIndex >= lVertexCount)
					continue;

				double lWeight = lCluster->GetControlPointWeights()[k];

				if (lWeight == 0.0)
				{
					continue;
				}

				// Compute the influence of the link on the vertex.
				FbxAMatrix lInfluence = lVertexTransformMatrix;
				MatrixScale(lInfluence, lWeight);

				if (lClusterMode == FbxCluster::eAdditive)
				{    
					// Multiply with the product of the deformations on the vertex.
					MatrixAddToDiagonal(lInfluence, 1.0 - lWeight);
					lClusterDeformation[lIndex] = lInfluence * lClusterDeformation[lIndex];

					// Set the link to 1.0 just to know this vertex is influenced by a link.
					lClusterWeight[lIndex] = 1.0;
				}
				else // lLinkMode == FbxCluster::eNormalize || lLinkMode == FbxCluster::eTotalOne
				{
					// Add to the sum of the deformations on the vertex.
					MatrixAdd(lClusterDeformation[lIndex], lInfluence);

					// Add to the sum of weights to either normalize or complete the vertex.
					lClusterWeight[lIndex] += lWeight;
				}
			}//For each vertex			
		}//lClusterCount
	}

	//Actually deform each vertices here by information stored in lClusterDeformation and lClusterWeight
	for (int i = 0; i < lVertexCount; i++) 
	{
		FbxVector4 lSrcVertex = pVertexArray[i];
		FbxVector4& lDstVertex = pVertexArray[i];
		double lWeight = lClusterWeight[i];

		// Deform the vertex if there was at least a link with an influence on the vertex,
		if (lWeight != 0.0) 
		{
			lDstVertex = lClusterDeformation[i].MultT(lSrcVertex);
			if (lClusterMode == FbxCluster::eNormalize)
			{
				// In the normalized link mode, a vertex is always totally influenced by the links. 
				lDstVertex /= lWeight;
			}
			else if (lClusterMode == FbxCluster::eTotalOne)
			{
				// In the total 1 link mode, a vertex can be partially influenced by the links. 
				lSrcVertex *= (1.0 - lWeight);
				lDstVertex += lSrcVertex;
			}
		} 
	}

	delete [] lClusterDeformation;
	delete [] lClusterWeight;
}

void ComputeSkinDeformation(FbxAMatrix& pGlobalPosition, 
	FbxMesh* pMesh, 
	FbxTime& pTime, 
	FbxVector4* pVertexArray,
	FbxPose* pPose)
{
	FbxSkin * lSkinDeformer = (FbxSkin *)pMesh->GetDeformer(0, FbxDeformer::eSkin);
	FbxSkin::EType lSkinningType = lSkinDeformer->GetSkinningType();

	if(lSkinningType == FbxSkin::eLinear || lSkinningType == FbxSkin::eRigid)
	{
		ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
	}
	else if(lSkinningType == FbxSkin::eDualQuaternion)
	{
		//ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, pVertexArray, pPose);
	}
	else if(lSkinningType == FbxSkin::eBlend)
	{
		int lVertexCount = pMesh->GetControlPointsCount();

		FbxVector4* lVertexArrayLinear = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayLinear, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		FbxVector4* lVertexArrayDQ = new FbxVector4[lVertexCount];
		memcpy(lVertexArrayDQ, pMesh->GetControlPoints(), lVertexCount * sizeof(FbxVector4));

		//ComputeLinearDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayLinear, pPose);
		//ComputeDualQuaternionDeformation(pGlobalPosition, pMesh, pTime, lVertexArrayDQ, pPose);

		// To blend the skinning according to the blend weights
		// Final vertex = DQSVertex * blend weight + LinearVertex * (1- blend weight)
		// DQSVertex: vertex that is deformed by dual quaternion skinning method;
		// LinearVertex: vertex that is deformed by classic linear skinning method;
		int lBlendWeightsCount = lSkinDeformer->GetControlPointIndicesCount();
		for(int lBWIndex = 0; lBWIndex<lBlendWeightsCount; ++lBWIndex)
		{
			double lBlendWeight = lSkinDeformer->GetControlPointBlendWeights()[lBWIndex];
			pVertexArray[lBWIndex] = lVertexArrayDQ[lBWIndex] * lBlendWeight + lVertexArrayLinear[lBWIndex] * (1 - lBlendWeight);
		}
	}
}

void ReadVertexCacheData(FbxMesh* pMesh, 
	FbxTime& pTime, 
	FbxVector4* pVertexArray)
{
	FbxVertexCacheDeformer* lDeformer     = static_cast<FbxVertexCacheDeformer*>(pMesh->GetDeformer(0, FbxDeformer::eVertexCache));
	FbxCache*               lCache        = lDeformer->GetCache();
	int                      lChannelIndex = -1;
	unsigned int             lVertexCount  = (unsigned int)pMesh->GetControlPointsCount();
	bool                     lReadSucceed  = false;
	double*                  lReadBuf      = new double[3*lVertexCount];

	if (lCache->GetCacheFileFormat() == FbxCache::eMayaCache)
	{
		if ((lChannelIndex = lCache->GetChannelIndex(lDeformer->GetCacheChannel())) > -1)
		{
			lReadSucceed = lCache->Read(lChannelIndex, pTime, lReadBuf, lVertexCount);
		}
	}
	else // eMaxPointCacheV2
	{
		lReadSucceed = lCache->Read((unsigned int)pTime.GetFrameCount(), lReadBuf, lVertexCount);
	}

	if (lReadSucceed)
	{
		unsigned int lReadBufIndex = 0;

		while (lReadBufIndex < 3*lVertexCount)
		{
			// In statements like "pVertexArray[lReadBufIndex/3].SetAt(2, lReadBuf[lReadBufIndex++])", 
			// on Mac platform, "lReadBufIndex++" is evaluated before "lReadBufIndex/3". 
			// So separate them.
			pVertexArray[lReadBufIndex/3].mData[0] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
			pVertexArray[lReadBufIndex/3].mData[1] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
			pVertexArray[lReadBufIndex/3].mData[2] = lReadBuf[lReadBufIndex]; lReadBufIndex++;
		}
	}

	delete [] lReadBuf;
}

SkeletalMesh::SkeletalMesh(void)
{
}


SkeletalMesh::~SkeletalMesh(void)
{
}

bool SkeletalMesh::ImportFromFbxMesh( FbxMesh* Mesh, FbxFileImporter* Importer )
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

	// If it has some defomer connection, update the vertices position
	const bool lHasVertexCache = Mesh->GetDeformerCount(FbxDeformer::eVertexCache) &&
		(static_cast<FbxVertexCacheDeformer*>(Mesh->GetDeformer(0, FbxDeformer::eVertexCache)))->IsActive();
	const bool lHasShape = Mesh->GetShapeCount() > 0;
	const bool lHasSkin = Mesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
	const bool lHasDeformation = lHasVertexCache || lHasShape || lHasSkin;
	
	const int lVertexCount1 = Mesh->GetControlPointsCount();
	FbxPose * lPose = NULL;
	lPose = Importer->mScene->GetPose(0);
	FbxAMatrix pGlobalPosition;
	FbxTimeSpan lTimeLineTimeSpan;
	Importer->mScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
	FbxVector4* lVertexArray = NULL;
	if (lHasDeformation)
	{
		lVertexArray = new FbxVector4[lVertexCount1];
		memcpy(lVertexArray, Mesh->GetControlPoints(), lVertexCount1 * sizeof(FbxVector4));
	}
	if (lHasDeformation)
	{
		// Active vertex cache deformer will overwrite any other deformer
		if (lHasVertexCache)
		{
			ReadVertexCacheData(Mesh, lTimeLineTimeSpan.GetStart(), lVertexArray);
		}
		else
		{
			if (lHasShape)
			{
				// Deform the vertex array with the shapes.
				//ComputeShapeDeformation(lMesh, pTime, pAnimLayer, lVertexArray);
			}

			//we need to get the number of clusters
			const int lSkinCount = Mesh->GetDeformerCount(FbxDeformer::eSkin);
			int lClusterCount = 0;
			for (int lSkinIndex = 0; lSkinIndex < lSkinCount; ++lSkinIndex)
			{
				lClusterCount += ((FbxSkin *)(Mesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin)))->GetClusterCount();
			}
			if (lClusterCount)
			{
				// Deform the vertex array with the skin deformer.
				ComputeSkinDeformation(pGlobalPosition, Mesh, lTimeLineTimeSpan.GetStart(), lVertexArray, lPose);
			}
		}

		//if (lMeshCache)
		//	lMeshCache->UpdateVertexPosition(lMesh, lVertexArray);
	}

	delete [] lVertexArray;

	//

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
		bd.ByteWidth = sizeof( NormalVertexGpuSkin ) * lPolygonVertexCount;
		NormalVertexGpuSkin* Vertices = new NormalVertexGpuSkin[lPolygonVertexCount];
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
		bd.ByteWidth = sizeof( NormalTexVertexGpuSkin ) * lPolygonVertexCount;
		NormalTexVertexGpuSkin* Vertices = new NormalTexVertexGpuSkin[lPolygonVertexCount];
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
		_VertexStride = sizeof(NormalVertexGpuSkin);
		_NumTexCoord = 0;
	}
	else if(_NormalArray != NULL && _TexCoordArray != NULL)
	{
		_VertexStride = sizeof(NormalTexVertexGpuSkin);
		_NumTexCoord = 1;
	}

	return true;
}
