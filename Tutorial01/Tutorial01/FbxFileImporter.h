#pragma once
#include <fbxsdk.h>
#include <string>
#include <vector>
#include <map>

class StaticMesh;
class SkeletalMesh;

struct BoneIndexInfo
{
	std::string BoneName;
	int Index;
	bool IsUsedLink;
	BoneIndexInfo(std::string Name, int idx):
		BoneName(Name),
		Index(idx),
		IsUsedLink(false)
	{
	}
	
	bool operator<(const BoneIndexInfo& other) const        
	{             
		if( Index < other.Index )
			return true;
		else
			return false;
	};
};

class FbxFileImporter
{
public:
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
	
	std::map<std::string, BoneIndexInfo> BoneIndexMap;
	std::vector<BoneIndexInfo> BoneArray;
	mutable Status mStatus;
	std::string FilePath;
public:
	void TriangulateRecursive(FbxNode* pNode);
	void FillFbxMeshArray(FbxNode* pNode, std::vector<StaticMesh*>& outStaticMeshArray);
	void FillFbxSkelMeshArray(FbxNode* pNode, std::vector<SkeletalMesh*>& outSkeletalMeshArray);

	void ImportStaticMesh(std::vector<StaticMesh*>& outStaticMeshArray);
	void ImportSkeletalMesh(std::vector<SkeletalMesh*>& outSkeletalMeshArray);

	FbxFileImporter( std::string Path);
	~FbxFileImporter(void);
};

