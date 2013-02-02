#pragma once
#include "camera.h"
class FpsCamera :
	public Camera
{
	XMFLOAT3 _Pos;
	XMFLOAT3 _Dir;

	float _Yaw;
	float _Pitch;
	
	float _CamRotSpeed;
	float _CamMoveSpeed;
public:
	virtual void Tick(float DeltaSeconds);
	virtual void CalcViewInfo(XMMATRIX& ViewMat, XMMATRIX& ProjectionMat, float Width, float Height);

	FpsCamera(XMFLOAT3 Pos, float Yaw, float Pitch);
	virtual ~FpsCamera(void);
};

