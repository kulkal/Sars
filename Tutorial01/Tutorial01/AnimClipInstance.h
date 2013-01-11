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
	void GetCurrentPose(SkeletonPose& InPose, float CurrentTime);

	AnimClipInstance(void);
	virtual ~AnimClipInstance(void);
};

