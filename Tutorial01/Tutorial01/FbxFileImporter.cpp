#include <windows.h>

#include "SkeletalMesh.h"
#include "StaticMesh.h"
#include "FbxFileImporter.h"

void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	pManager = FbxManager::Create();
	if( !pManager )
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(pManager, "My Scene");
	if( !pScene )
	{
		FBXSDK_printf("Error: Unable to create FBX scene!\n");
		exit(1);
	}
}

FbxFileImporter::FbxFileImporter( std::string Path)
	:mSdkManager(NULL),
	mScene(NULL),
	mImporter(NULL),
	FilePath(Path),
	mStatus(UNLOADED)
{
	InitializeSdkObjects(mSdkManager, mScene);

	if (mSdkManager)
	{
		// Create the importer.
		int lFileFormat = -1;
		mImporter = FbxImporter::Create(mSdkManager,"");
		if (!mSdkManager->GetIOPluginRegistry()->DetectReaderFileFormat(FilePath.c_str(), lFileFormat) )
		{
			// Unrecognizable file format. Try to fall back to FbxImporter::eFBX_BINARY
			lFileFormat = mSdkManager->GetIOPluginRegistry()->FindReaderIDByDescription( "FBX binary (*.fbx)" );;
		}

		// Initialize the importer by providing a filename.
		if(mImporter->Initialize(FilePath.c_str(), lFileFormat) == true)
		{
			// The file is going to be imported at 
			// the end of the first display callback.
			//mWindowMessage = "Importing file ";
		//	mWindowMessage += mFileName;
			//mWindowMessage += "\nPlease wait!";

			// Set scene status flag to ready to load.
			mStatus = MUST_BE_LOADED;
		}
		else
		{
			//mWindowMessage = "Unable to open file ";
			//mWindowMessage += mFileName;
		//	mWindowMessage += "\nError reported: ";
		//	mWindowMessage += mImporter->GetLastErrorString();
		//	mWindowMessage += "\nEsc to exit";
		}
	}
	else
	{
	//	mWindowMessage = "Unable to create the FBX SDK manager";
	//	mWindowMessage += "\nEsc to exit";
	}
}


FbxFileImporter::~FbxFileImporter(void)
{
   FbxArrayDelete(mAnimStackNameArray);

    // Unload the cache and free the memory

    // Delete the FBX SDK manager. All the objects that have been allocated 
    // using the FBX SDK manager and that haven't been explicitly destroyed 
    // are automatically destroyed at the same time.
   if(mScene)mScene->Destroy();
   if(mImporter)mImporter->Destroy();
    if( mSdkManager ) mSdkManager->Destroy();
}

void FbxFileImporter::ImportStaticMesh(std::vector<StaticMesh*>& outStaticMeshArray)
{
	bool lResult = false;
	// Make sure that the scene is ready to load.
	if (mStatus == MUST_BE_LOADED)
	{
		if (mImporter->Import(mScene) == true)
		{
			// Set the scene status flag to refresh 
			// the scene in the first timer callback.
			mStatus = MUST_BE_REFRESHED;

			// Convert Axis System to what is used in this example, if needed
			FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
		//	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eYAxis, FbxAxisSystem::eRightHanded);

			FbxAxisSystem::EFrontVector FrontVector = (FbxAxisSystem::EFrontVector)-FbxAxisSystem::eParityOdd;
			const FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FrontVector, FbxAxisSystem::eLeftHanded);

			if( SceneAxisSystem != OurAxisSystem )
			{
				FbxRootNodeUtility::RemoveAllFbxRoots( mScene );
				//OurAxisSystem.ConvertScene(mScene);
			}

			// Convert Unit System to what is used in this example, if needed
			FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
			if( SceneSystemUnit.GetScaleFactor() != 1.0 )
			{
				//The unit in this example is centimeter.
				FbxSystemUnit::cm.ConvertScene( mScene);
			}

			// Get the list of all the animation stack.
			mScene->FillAnimStackNameArray(mAnimStackNameArray);
			
			//TriangulateRecursive(mScene->GetRootNode());
			FillFbxMeshArray(mScene->GetRootNode(), outStaticMeshArray);
			

			lResult = true;
		}
		else
		{
			// Import failed, set the scene status flag accordingly.
			mStatus = UNLOADED;
		}

		// Destroy the importer to release the file.
		mImporter->Destroy();
		mImporter = NULL;
	}
}

void FbxFileImporter::FillFbxMeshArray( FbxNode* pNode, std::vector<StaticMesh*>& outStaticMeshArray )
{
	FbxNodeAttribute* NodeAttribute = pNode->GetNodeAttribute();
	if ( NodeAttribute )
	{
		if (NodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh * pFbxMesh = pNode->GetMesh();
			if (pFbxMesh)
			{
				StaticMesh* pStaticMesh = new StaticMesh;
				pStaticMesh->ImportFromFbxMesh(pFbxMesh, this);
				outStaticMeshArray.push_back(pStaticMesh);
			}
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FillFbxMeshArray(pNode->GetChild(lChildIndex), outStaticMeshArray);
	}
}

void FbxFileImporter::TriangulateRecursive(FbxNode* pNode)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter lConverter(pNode->GetFbxManager());
			lConverter.TriangulateInPlace(pNode);
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		TriangulateRecursive(pNode->GetChild(lChildIndex));
	}
}

void FillBoneIndexMapRecursive(FbxNode* Node, std::map<std::string, BoneIndexInfo>& BoneIndexMap, int& NumBone)
{
	BoneIndexMap.insert(std::pair<std::string, BoneIndexInfo>(Node->GetName(), BoneIndexInfo(Node->GetName(), NumBone)));
	NumBone++;

	const int lChildCount = Node->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FillBoneIndexMapRecursive(Node->GetChild(lChildIndex), BoneIndexMap, NumBone);
	}
}

void FbxFileImporter::ImportSkeletalMesh( std::vector<SkeletalMesh*>& outSkeletalMeshArray )
{
	bool lResult = false;
	// Make sure that the scene is ready to load.
	if (mStatus == MUST_BE_LOADED)
	{
		if (mImporter->Import(mScene) == true)
		{
			// Set the scene status flag to refresh 
			// the scene in the first timer callback.
			mStatus = MUST_BE_REFRESHED;

			// Convert Axis System to what is used in this example, if needed
			FbxAxisSystem SceneAxisSystem = mScene->GetGlobalSettings().GetAxisSystem();
			//	FbxAxisSystem OurAxisSystem(FbxAxisSystem::eZAxis, FbxAxisSystem::eYAxis, FbxAxisSystem::eRightHanded);

			FbxAxisSystem::EFrontVector FrontVector = (FbxAxisSystem::EFrontVector)-FbxAxisSystem::eParityOdd;
			const FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FrontVector, FbxAxisSystem::eLeftHanded);

			if( SceneAxisSystem != OurAxisSystem )
			{
				FbxRootNodeUtility::RemoveAllFbxRoots( mScene );
				//OurAxisSystem.ConvertScene(mScene);
			}

			// Convert Unit System to what is used in this example, if needed
			FbxSystemUnit SceneSystemUnit = mScene->GetGlobalSettings().GetSystemUnit();
			if( SceneSystemUnit.GetScaleFactor() != 1.0 )
			{
				//The unit in this example is centimeter.
				FbxSystemUnit::cm.ConvertScene( mScene);
			}

			// Get the list of all the animation stack.
			mScene->FillAnimStackNameArray(mAnimStackNameArray);
			

			int NumBone = 0;
			FillBoneIndexMapRecursive(mScene->GetRootNode(), BoneIndexMap, NumBone);

		/*	std::map<std::string, BoneIndexInfo>::iterator it;
			for(it=BoneIndexMap.begin();it!=BoneIndexMap.end();it++)
			{
				std::string BoneName = it->first;
				BoneIndexInfo& BoneIndexInfo = it->second;
			}*/

			//TriangulateRecursive(mScene->GetRootNode());
			FillFbxSkelMeshArray(mScene->GetRootNode(), outSkeletalMeshArray);

			//ImportSkeleton(&outSkeletalMeshArray[0]->_Skeleton, &outSkeletalMeshArray[0]->_Pose);


			//FbxAnimStack * lCurrentAnimationStack = mScene->FindMember<FbxAnimStack>(mAnimStackNameArray[0]->Buffer());
			//if (lCurrentAnimationStack == NULL)
			//{
			//	// this is a problem. The anim stack should be found in the scene!
			//	return;
			//}

			//// we assume that the first animation layer connected to the animation stack is the base layer
			//// (this is the assumption made in the FBXSDK)
			//FbxAnimLayer* mCurrentAnimLayer = lCurrentAnimationStack->GetMember<FbxAnimLayer>();
			//mScene->GetEvaluator()->SetContext(lCurrentAnimationStack);

			//int PoseCount = mScene->GetPoseCount();

			//lCurrentAnimationStack->Destroy();

			lResult = true;
		}
		else
		{
			// Import failed, set the scene status flag accordingly.
			mStatus = UNLOADED;
		}

		// Destroy the importer to release the file.
		mImporter->Destroy();
		mImporter = NULL;
	}
}

void FbxFileImporter::FillFbxSkelMeshArray( FbxNode* pNode, std::vector<SkeletalMesh*>& outSkeletalMeshArray )
{
	FbxNodeAttribute* NodeAttribute = pNode->GetNodeAttribute();
	if ( NodeAttribute )
	{
		if (NodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			FbxMesh * pFbxMesh = pNode->GetMesh();
			if (pFbxMesh)
			{
				SkeletalMesh* Mesh = new SkeletalMesh;
				Mesh->ImportFromFbxMesh(pFbxMesh, this);
				outSkeletalMeshArray.push_back(Mesh);
			}
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FillFbxSkelMeshArray(pNode->GetChild(lChildIndex), outSkeletalMeshArray);
	}
}

void FbxFileImporter::FillFbxNodeArray(FbxNode* pNode, std::vector<FbxNode*>& outNodeArray)
{

	outNodeArray.push_back(pNode);

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FillFbxNodeArray(pNode->GetChild(lChildIndex), outNodeArray);
	}
}

void FbxFileImporter::FillFbxClusterArray(FbxNode* pNode, std::vector<FbxCluster*>& outClusterArray)
{
	FbxMesh * pFbxMesh = pNode->GetMesh();
	if (pFbxMesh)
	{
		const int lSkinCount = pFbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		if(lSkinCount > 0)
		{
			for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
			{
				FbxSkin * lSkinDeformer = (FbxSkin *)pFbxMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
				int lClusterCount = lSkinDeformer->GetClusterCount();
				for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
				{
					FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
					if (!lCluster->GetLink())
						continue;
					
					outClusterArray.push_back(lCluster);
				}
			}

		}
	}
	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FillFbxClusterArray(pNode->GetChild(lChildIndex), outClusterArray);
	}
}


void FbxFileImporter::ImportSkeleton(Skeleton** OutSkeleton, SkeletonPose** OutRefPose)
{
	std::vector<FbxNode*> NodeArray;
	std::vector<FbxCluster*> ClusterArray;
	std::vector<FbxAMatrix> GlobalMatArray;

	FillFbxNodeArray(mScene->GetRootNode(), NodeArray);
	FillFbxClusterArray(mScene->GetRootNode(), ClusterArray);

	
	SkeletonJoint* Joints = new SkeletonJoint[NodeArray.size()];
	JointPose* RefPose = new JointPose[NodeArray.size()];

	GlobalMatArray.resize(NodeArray.size());
	for(unsigned int NodeIndex=0;NodeIndex<NodeArray.size();NodeIndex++)
	{
		FbxNode* Node = NodeArray[NodeIndex];

		GlobalMatArray[NodeIndex] = Node->EvaluateGlobalTransform();
	}

	for(unsigned int NodeIndex=0;NodeIndex<NodeArray.size();NodeIndex++)
	{
		FbxNode* Node = NodeArray[NodeIndex];
		SkeletonJoint& Joint = Joints[NodeIndex];
		Joint._Name = Node->GetName();
		// finding parent index
		for(unsigned int OtherNodeIndex=0;OtherNodeIndex<NodeArray.size();OtherNodeIndex++)
		{
			FbxNode* OtherNode = NodeArray[OtherNodeIndex];
			if(Node->GetParent() )
			{
				
			}

			if(Node->GetParent() == OtherNode)
			{
				Joint._ParentIndex = OtherNodeIndex;
			}
		}
	}

	for(unsigned int NodeIndex=0;NodeIndex<NodeArray.size();NodeIndex++)
	{
		FbxNode* Node = NodeArray[NodeIndex];
		SkeletonJoint& Joint = Joints[NodeIndex];
		Joint._Name = Node->GetName();
		JointPose& RefPosJoint = RefPose[NodeIndex];
		
		bool IsBoneCluser = false;
		for(unsigned int ClusterIndex=0;ClusterIndex<ClusterArray.size();ClusterIndex++)
		{
			FbxCluster* Cluster = ClusterArray[ClusterIndex];

			if(Node == Cluster->GetLink())
			{
				// this node has cluster, so it has bind pose
				FbxAMatrix BoneGlobalBind, ClusterMatrix;
				Cluster->GetTransformLinkMatrix(BoneGlobalBind);
				Cluster->GetTransformMatrix(ClusterMatrix);
				BoneGlobalBind = ClusterMatrix.Inverse() * BoneGlobalBind;

				FbxAMatrix BoneGlobalBindInv = BoneGlobalBind.Inverse();
				// ref inverse
				FbxVector4 S = BoneGlobalBindInv.GetS();
				FbxVector4 T = BoneGlobalBindInv.GetT();
				FbxQuaternion Q = BoneGlobalBindInv.GetQ();

				XMFLOAT4 QQ;
				QQ.x = (float)Q[0];
				QQ.y = (float)Q[1];
				QQ.z = (float)Q[2];
				QQ.w = (float)Q[3];
				XMVECTOR Quat = XMLoadFloat4((XMFLOAT4*)&QQ);
				
				XMMATRIX MatRot = XMMatrixRotationQuaternion(Quat);
				XMMATRIX MatTrans = XMMatrixTranslation((float)T[0], (float)T[1], (float)T[2]);
				XMMATRIX MatScale = XMMatrixScaling((float)S[0], (float)S[1], (float)S[2]);

				XMMATRIX RefWorldInv = XMMatrixIdentity();
				RefWorldInv = XMMatrixMultiply(MatScale, MatRot);
				RefWorldInv = XMMatrixMultiply(RefWorldInv, MatTrans);

				XMStoreFloat4x4(&Joint._InvRefPose, RefWorldInv);
			
				// pose

				FbxAMatrix ParentGlobalPose;
				FbxAMatrix GlobalPose;
				if(Joint._ParentIndex >= 0)
					ParentGlobalPose = GlobalMatArray[Joint._ParentIndex] ;
				else
					ParentGlobalPose.SetIdentity();

				GlobalPose = Cluster->GetLink()->EvaluateGlobalTransform();;

				FbxAMatrix BoneMatLocalPose = ParentGlobalPose.Inverse() * GlobalPose;


				FbxVector4 LocalT = BoneMatLocalPose.GetT();
				FbxQuaternion LocalQ = BoneMatLocalPose.GetQ();
				FbxVector4 LocalS = BoneMatLocalPose.GetS();

				RefPosJoint._Rot.x = (float)LocalQ[0];
				RefPosJoint._Rot.y = (float)LocalQ[1];
				RefPosJoint._Rot.z = (float)LocalQ[2];
				RefPosJoint._Rot.w = (float)LocalQ[3];

				RefPosJoint._Trans.x = (float)LocalT[0];
				RefPosJoint._Trans.y = (float)LocalT[1];
				RefPosJoint._Trans.z = (float)LocalT[2];
				
				RefPosJoint._Scale.x = (float)LocalS[0];
				RefPosJoint._Scale.y = (float)LocalS[1];
				RefPosJoint._Scale.z = (float)LocalS[2];

				FbxVector4 GlobalT = ParentGlobalPose.GetT();

				IsBoneCluser = true;
			}
		}
		if( IsBoneCluser == false)
		{
				FbxAMatrix BoneGlobalBind;
				BoneGlobalBind = GlobalMatArray[NodeIndex];
				

				// ref inverse
				FbxVector4 S = BoneGlobalBind.GetS();
				FbxVector4 T = BoneGlobalBind.GetT();
				FbxQuaternion Q = BoneGlobalBind.GetQ();
				XMFLOAT4 QQ;
				QQ.x = (float)Q[0];
				QQ.y = (float)Q[1];
				QQ.z = (float)Q[2];
				QQ.w = (float)Q[3];
				XMVECTOR Quat = XMLoadFloat4((XMFLOAT4*)&QQ);
				
				XMMATRIX MatRot = XMMatrixRotationQuaternion(Quat);
				XMMATRIX MatTrans = XMMatrixTranslation((float)T[0], (float)T[1], (float)T[2]);
				XMMATRIX MatScale = XMMatrixScaling((float)S[0], (float)S[1], (float)S[2]);

				//XMMATRIX RefWorldInv = MatScale * MatRot * MatTrans;//XMMatrixMultiply(MatRot, MatTrans);
				XMMATRIX RefWorldInv =XMMatrixMultiply(XMMatrixMultiply( MatScale , MatRot), MatTrans);//XMMatrixMultiply(MatRot, MatTrans);

				XMVECTOR Det;
				RefWorldInv = XMMatrixInverse(&Det, RefWorldInv);

				XMStoreFloat4x4(&Joint._InvRefPose, RefWorldInv);
			
				// pose
				FbxAMatrix ParentGlobalPose;
				if(Joint._ParentIndex >= 0)
					ParentGlobalPose = GlobalMatArray[Joint._ParentIndex] ;
				else
					ParentGlobalPose.SetIdentity();
				FbxAMatrix BoneMatLocal = ParentGlobalPose.Inverse() * BoneGlobalBind;

				FbxVector4 LocalT = BoneMatLocal.GetT();
				FbxQuaternion LocalQ = BoneMatLocal.GetQ();
				FbxVector4 LocalS = BoneMatLocal.GetS();

				RefPosJoint._Rot.x = (float)LocalQ[0];
				RefPosJoint._Rot.y = (float)LocalQ[1];
				RefPosJoint._Rot.z = (float)LocalQ[2];
				RefPosJoint._Rot.w = (float)LocalQ[3];

				RefPosJoint._Trans.x = (float)LocalT[0];
				RefPosJoint._Trans.y = (float)LocalT[1];
				RefPosJoint._Trans.z = (float)LocalT[2];
				
				RefPosJoint._Scale.x = (float)LocalS[0];
				RefPosJoint._Scale.y = (float)LocalS[1];
				RefPosJoint._Scale.z = (float)LocalS[2];
				//char Str[128];
				//sprintf(Str, "%s is dummy bone, %d\n", Joint._Name.c_str(), NodeIndex);
				//OutputDebugStringA(Str);
		}
	}

	//Skeleton* NewSkeleton = new Skeleton;
	(*OutSkeleton)->_Joints = Joints;
	(*OutSkeleton)->_JointCount = NodeArray.size();
	//*OutSkeleton = NewSkeleton;

	//SkeletonPose* NewRefPose = new SkeletonPose;
	(*OutRefPose)->_LocalPoseArray = RefPose;
	//*OutRefPose = NewRefPose;

	return;
}

void FbxFileImporter::FillSkeletonJointRecursive( FbxNode* pNode, std::vector<SkeletonJoint>& outJounts )
{
	FbxMesh * pFbxMesh = pNode->GetMesh();
	if (pFbxMesh)
	{
		const int lSkinCount = pFbxMesh->GetDeformerCount(FbxDeformer::eSkin);
		if(lSkinCount > 0)
		{
			for ( int lSkinIndex=0; lSkinIndex<lSkinCount; ++lSkinIndex)
			{
				FbxSkin * lSkinDeformer = (FbxSkin *)pFbxMesh->GetDeformer(lSkinIndex, FbxDeformer::eSkin);
				int lClusterCount = lSkinDeformer->GetClusterCount();
				for ( int lClusterIndex=0; lClusterIndex<lClusterCount; ++lClusterIndex)
				{
					FbxCluster* lCluster = lSkinDeformer->GetCluster(lClusterIndex);
					if (!lCluster->GetLink())
						continue;
					


				}
			}

		}
	}
	SkeletonJoint Joint;



	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		FillSkeletonJointRecursive(pNode->GetChild(lChildIndex), outJounts);
	}
}
