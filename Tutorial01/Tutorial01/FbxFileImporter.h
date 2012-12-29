#pragma once
#include <fbxsdk.h>
#include <string>

class StaticMesh;

class FbxFileImporter
{
	FbxManager * mSdkManager;
	FbxScene * mScene;
	FbxImporter * mImporter;
public:

	StaticMesh* ImportStaticMesh();

	FbxFileImporter( std::string Path);
	~FbxFileImporter(void);
};

