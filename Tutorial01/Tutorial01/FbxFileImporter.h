#pragma once
#include <fbxsdk.h>
#include <string>
#include <vector>
class StaticMesh;

class FbxFileImporter
{
	enum Status
	{
		UNLOADED,               // Unload file or load failure;
		MUST_BE_LOADED,         // Ready for loading file;
		MUST_BE_REFRESHED,      // Something changed and redraw needed;
		REFRESHED               // No redraw needed.
	};

	FbxManager * mSdkManager;
	FbxScene * mScene;
	FbxImporter * mImporter;
	FbxArray<FbxString*> mAnimStackNameArray;
	FbxArray<FbxNode*> mCameraArray;
	FbxArray<FbxPose*> mPoseArray;
	FbxArray<FbxNode*> FbxMeshArray;


	mutable Status mStatus;
	std::string FilePath;
public:
	void TriangulateRecursive(FbxNode* pNode);
	void FillFbxMeshArray(FbxNode* pNode, std::vector<StaticMesh*>& outStaticMeshArray);

	void ImportStaticMesh(std::vector<StaticMesh*>& outStaticMeshArray);


	FbxFileImporter( std::string Path);
	~FbxFileImporter(void);
};

