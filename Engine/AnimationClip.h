#pragma once
#include "BaseObject.h"
#include "Skeleton.h"

struct TranslationTrack
{
	std::vector<XMFLOAT3>	_PosArray;
	std::vector<float>		_TimeArray;

	void GetPos(XMFLOAT3* OutPos, float NormalizedTime, float LocalTime);
};

struct RotationTrack
{
	std::vector<XMFLOAT4>	_RotArray;
	std::vector<float>		_TimeArray;

	void GetRot(XMFLOAT4* OutRot, float NormalizedTime, float LocalTime);
};

struct ScaleTrack
{
	std::vector<XMFLOAT3>	_ScaleArray;
	std::vector<float>		_TimeArray;

	void GetScale(XMFLOAT3* OutPos, float NormalizedTime, float LocalTime);
};

class AnimationClip :
	public BaseObject
{
	friend class FbxFileImporter;
	friend class AnimClipInstance;
	float _Duration;
	std::vector<TranslationTrack> _TransTrackArray;
	std::vector<RotationTrack> _RotTrackArray;
	std::vector<ScaleTrack> _ScaleTrackArray;

public:

	void GetCurrentPose(SkeletonPose& InPose, float CurrentTime);

	AnimationClip(void);
	virtual ~AnimationClip(void);
};

