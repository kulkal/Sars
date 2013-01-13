#pragma once
#include <fbxsdk.h>
#include <string>
#include <vector>
#include <map>
#include "Skeleton.h"
class StaticMesh;
class SkeletalMesh;
class AnimationClip;


struct BoneIndexInfo
{
	std::string BoneName;
	int SkeletonIndex;
	int Index;
	bool IsUsedLink;
	BoneIndexInfo(std::string Name, int idx):
		BoneName(Name),
		Index(idx),
		SkeletonIndex(idx),
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

	FbxTime mFrameTime;
	FbxTime mStart;
	FbxTime mStop;
	FbxTime mCurrentTime;
public:
	void TriangulateRecursive(FbxNode* pNode);
	void FillFbxMeshArray(FbxNode* pNode, std::vector<StaticMesh*>& outStaticMeshArray);
	void FillFbxSkelMeshArray(FbxNode* pNode, std::vector<SkeletalMesh*>& outSkeletalMeshArray);
	void FillFbxNodeArray(FbxNode* pNode, std::vector<FbxNode*>& outNodeArray);
	void FillFbxClusterArray(FbxNode* pNode, std::vector<FbxCluster*>& outClusterArray);

	void ImportSkeleton(Skeleton** OutSkeleton, SkeletonPose** OutRefPose);
	void FillSkeletonJointRecursive(FbxNode* pNode, std::vector<SkeletonJoint>& outJounts);

	void ImportStaticMesh(std::vector<StaticMesh*>& outStaticMeshArray);
	void ImportSkeletalMesh(std::vector<SkeletalMesh*>& outSkeletalMeshArray);

	void ImportAnimClip(std::vector<AnimationClip*>& outAnimclipArray);
	void FillAnimTrackRecursive(FbxNode* pNode, AnimationClip* Clip);
	void SampleCurrentRecursive(FbxNode* pNode, AnimationClip* Clip, int NodeIndex);

	FbxFileImporter( std::string Path);
	~FbxFileImporter(void);
};

