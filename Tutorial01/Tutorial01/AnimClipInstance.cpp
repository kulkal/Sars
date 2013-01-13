#include "AnimClipInstance.h"


AnimClipInstance::AnimClipInstance(AnimationClip* InClip)
	:_Clip(InClip)
	,_LocalTime(0.f)
	,_TimeScale(1.f)
	,_StartTime(0.f)
{
}


AnimClipInstance::~AnimClipInstance(void)
{
}

void AnimClipInstance::GetCurrentPose(SkeletonPose& InPose)
{
	_Clip->GetCurrentPose(InPose, _LocalTime);
}

void AnimClipInstance::SetCurrentTime( float InCurrentTime )
{
	_LocalTime = fmod((InCurrentTime - _StartTime),_Clip->_Duration);
}

void AnimClipInstance::Play()
{

}

void AnimClipInstance::Stop()
{

}
