#include "AnimClipInstance.h"
#include "MathUtil.h"
#include "OutputDebug.h"
#include "Engine.h"
#define FLOAT_MAX  3.40282e+038
AnimClipInstance::AnimClipInstance(AnimationClip* InClip)
	:_Clip(InClip)
	,_LocalTime(0.f)
	,_TimeScale(1.f)
	,_StartTime(0.f)
	,_NumPlay(0)
{
}


AnimClipInstance::~AnimClipInstance(void)
{
}

void AnimClipInstance::GetCurrentPose(SkeletonPose& InPose)
{
	_Clip->GetCurrentPose(InPose, _LocalTime);
}

void AnimClipInstance::Tick()
{
	float CurrentGlobalTime = GEngine->_TimeSeconds;
	float NT;
	if(_NumPlay == 0)
	{
		NT = FLOAT_MAX;
	}
	else
	{
		NT = _Clip->_Duration*(float)_NumPlay;
	}

	_LocalTime = fmod(Math::Clamp<float>((CurrentGlobalTime - _StartTime)*_TimeScale, -NT, NT), _Clip->_Duration);

	if(_LocalTime < 0)
	{
		_LocalTime = _Clip->_Duration + _LocalTime;
	}
}

void AnimClipInstance::Play(int InNumPlay)
{
	_NumPlay = InNumPlay;
	_StartTime = GEngine->_TimeSeconds;
}

void AnimClipInstance::Stop()
{

}
