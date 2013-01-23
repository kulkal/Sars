#include "AnimationClip.h"
#include "OutputDebug.h"

AnimationClip::AnimationClip(void)
	:_Duration(0.f)
{
}


AnimationClip::~AnimationClip(void)
{
	_TransTrackArray.clear();
	_RotTrackArray.clear();
	_ScaleTrackArray.clear();
}

void AnimationClip::GetCurrentPose(SkeletonPose& InPose, float CurrentTime)
{
	float NormalizedTime = CurrentTime / _Duration;
	for(unsigned int i=0;i<_TransTrackArray.size();i++)
	{
		JointPose& Joint = InPose._LocalPoseArray[i];

		ScaleTrack& STrack = _ScaleTrackArray[i];
		STrack.GetScale(&Joint._Scale, NormalizedTime, CurrentTime);

		RotationTrack& RTrack = _RotTrackArray[i];
		RTrack.GetRot(&Joint._Rot, NormalizedTime, CurrentTime);


		TranslationTrack& TTrack = _TransTrackArray[i];
		TTrack.GetPos(&Joint._Trans, NormalizedTime, CurrentTime);
	}
}

void FindKeyIndices(std::vector<float>& TimeArray, float NormalizedTime, float LocalTime, int& KeyIndex0, int& KeyIndex1, float& Alpha)
{
	int EstimatedKeyIndex = TimeArray.size() * NormalizedTime;
	if(EstimatedKeyIndex == 0)
	{
		KeyIndex1 = 1;//TimeArray.size()-1;
		KeyIndex0 = 0;//KeyIndex1 - 1;
	}
	else
	{
		while(TimeArray[EstimatedKeyIndex] >= LocalTime)
		{
			EstimatedKeyIndex--;
		}
	
		KeyIndex0 = EstimatedKeyIndex;
		KeyIndex1 = EstimatedKeyIndex + 1;
	}

	Alpha = (LocalTime - TimeArray[KeyIndex0])/(TimeArray[KeyIndex1] - TimeArray[KeyIndex0]);
}

void TranslationTrack::GetPos(XMFLOAT3* OutPos, float NormalizedTime, float LocalTime)
{
	int KeyIndex0, KeyIndex1;
	float Alpha;
	
	FindKeyIndices(_TimeArray, NormalizedTime, LocalTime, KeyIndex0, KeyIndex1, Alpha);
	
	XMStoreFloat3(OutPos, XMVectorLerp(XMLoadFloat3(&_PosArray[KeyIndex0]), XMLoadFloat3(&_PosArray[KeyIndex1]), Alpha));
}

void RotationTrack::GetRot(XMFLOAT4* OutRot, float NormalizedTime, float LocalTime)
{
	int KeyIndex0, KeyIndex1;
	float Alpha;

	FindKeyIndices(_TimeArray, NormalizedTime, LocalTime, KeyIndex0, KeyIndex1, Alpha);
	XMStoreFloat4(OutRot, XMQuaternionSlerp(XMLoadFloat4(&_RotArray[KeyIndex0]), XMLoadFloat4(&_RotArray[KeyIndex1]), Alpha));
}

void ScaleTrack::GetScale(XMFLOAT3* OutPos, float NormalizedTime, float LocalTime)
{
	int KeyIndex0, KeyIndex1;
	float Alpha;
	
	FindKeyIndices(_TimeArray, NormalizedTime, LocalTime, KeyIndex0, KeyIndex1, Alpha);
	
	XMStoreFloat3(OutPos, XMVectorLerp(XMLoadFloat3(&_ScaleArray[KeyIndex0]), XMLoadFloat3(&_ScaleArray[KeyIndex1]), Alpha));
}
