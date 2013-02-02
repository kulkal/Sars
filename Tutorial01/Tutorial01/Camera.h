#pragma once
#include "baseobject.h"
class Camera :
	public BaseObject
{
protected:
	float _Near;
	float _Far;
public:
	virtual void Tick(float DeltaSeconds){DeltaSeconds;}
	virtual void CalcViewInfo(XMMATRIX& ViewMat, XMMATRIX& ProjectionMat, float Width, float Height){ViewMat;ProjectionMat;Width;Height;}
	Camera(void);
	virtual ~Camera(void);
};

