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
	int _NumPlay;
	AnimationClip* _Clip;
public:
	void Play(int InNumPlay);
	void Stop();
	void SetTimeScale(float InScale){_TimeScale = InScale;}
	void Tick();
	void GetCurrentPose(SkeletonPose& InPose);

	AnimClipInstance(AnimationClip* InClip);
	virtual ~AnimClipInstance(void);
};

