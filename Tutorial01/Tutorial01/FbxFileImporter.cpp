#include <windows.h>

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
			for(int i=0;i<mAnimStackNameArray.Size();i++)
			{
				char* AnimStackname = mAnimStackNameArray[i]->Buffer();
				OutputDebugStringA(AnimStackname);
				OutputDebugStringA("\n");
			}
			
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
	OutputDebugStringA(pNode->GetName());
	OutputDebugStringA("\n");
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