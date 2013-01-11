#pragma once
#include "BaseObject.h"
#include "Skeleton.h"

struct TranslationTrack
{
	std::vector<XMFLOAT3>	_PosArray;
	std::vector<float>		_TimeArray;
};

struct RotationTrack
{
	std::vector<XMFLOAT4>	_RotArray;
	std::vector<float>		_TimeArray;
};

struct ScaleTrack
{
	std::vector<XMFLOAT3>	_ScaleArray;
	std::vector<float>		_TimeArray;
};

class AnimationClip :
	public BaseObject
{
	float _Duration;
	std::vector<TranslationTrack> _TransTrackArray;
	std::vector<RotationTrack> _RotTrackArray;
	std::vector<ScaleTrack> _TrackTrackArray;

public:

	void GetCurrentPose(SkeletonPose& InPose, float CurrentTime);

	AnimationClip(void);
	virtual ~AnimationClip(void);
};

