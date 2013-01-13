#pragma once
#include "baseobject.h"
#include "AnimationClip.h"
#include "Skeleton.h"

class AnimClipInstance :
	public BaseObject
{
	float _TimeScale;
	float _StartTime;
	float _LocalTime;
	AnimationClip* _Clip;
public:
	void Play();
	void Stop();
	void SetTimeScale(float InScale){_TimeScale = InScale;}
	void SetCurrentTime(float InCurrentTime);
	void GetCurrentPose(SkeletonPose& InPose);

	AnimClipInstance(AnimationClip* InClip);
	virtual ~AnimClipInstance(void);
};

