#pragma once
#include "baseobject.h"
#include "AnimationClip.h"
#include "Skeleton.h"

class AnimClipInstance :
	public BaseObject
{
	float _StartTime;
	AnimationClip* _Clip;
public:
	void Play();
	void GetCurrentPose(SkeletonPose& InPose, float CurrentTime);

	AnimClipInstance(AnimationClip* InClip);
	virtual ~AnimClipInstance(void);
};

