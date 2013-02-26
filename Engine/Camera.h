#pragma once
#include "baseobject.h"
class Camera :
	public BaseObject
{
protected:
	float _Near;
	float _Far;
public:
	float GetNear(){return _Near;}
	float GetFar(){return _Far;}
	virtual void Tick(float DeltaSeconds){DeltaSeconds;}
	virtual void CalcViewInfo(XMMATRIX& ViewMat, XMMATRIX& ProjectionMat, float Width, float Height){ViewMat;ProjectionMat;Width;Height;}
	Camera(float Near, float Far);
	virtual ~Camera(void);
};

