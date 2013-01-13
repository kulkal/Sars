#include "AnimClipInstance.h"


AnimClipInstance::AnimClipInstance(AnimationClip* InClip)
	:_Clip(InClip)
{
}


AnimClipInstance::~AnimClipInstance(void)
{
}

void AnimClipInstance::GetCurrentPose(SkeletonPose& InPose, float CurrentTime)
{
	float LocalTime = CurrentTime - _StartTime;
}
