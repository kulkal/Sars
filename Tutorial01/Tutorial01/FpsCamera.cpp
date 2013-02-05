#include "FpsCamera.h"
#include "Input.h"
#include "Engine.h"

#define SMALLFLOAT 0.00000000001f
FpsCamera::FpsCamera(XMFLOAT3 Pos, float Yaw, float Pitch)
	:Camera(10.f, 2000.f)
	,_Pos(Pos)
	,_Yaw(Yaw)
	,_Pitch(Pitch)
{
	_CamRotSpeed = 10.f;
	_CamMoveSpeed = 100.f;
}


FpsCamera::~FpsCamera(void)
{
}

void FpsCamera::Tick( float DeltaSeconds )
{
	_Yaw -= (float)GEngine->_Input->m_lDX * DeltaSeconds * _CamRotSpeed;
	_Pitch -= (float)GEngine->_Input->m_lDY * DeltaSeconds * _CamRotSpeed;

	XMMATRIX CamRotMat = XMMatrixRotationX(_Pitch) * XMMatrixRotationY(_Yaw);

	XMVECTOR Dir = XMVectorSet(0.f, 0.f, -1.f, 0.f);
	Dir = XMVector3Transform(Dir, CamRotMat);

	XMVECTOR DirX = XMVectorSet(-1.f, 0.f, 0.f, 0.f);
	DirX = XMVector3Transform(DirX, CamRotMat);
	
	if(GEngine->_Input->IsKeyHeldDn(DIK_W))
	{
		XMVECTOR Vel = Dir * DeltaSeconds * _CamMoveSpeed;
		XMVECTOR NewPos = XMLoadFloat3(&_Pos) + Vel;
		XMStoreFloat3(&_Pos, NewPos);
	}
	else if(GEngine->_Input->IsKeyHeldDn(DIK_S))
	{
		XMVECTOR Vel = Dir * DeltaSeconds * _CamMoveSpeed;
		XMVECTOR NewPos = XMLoadFloat3(&_Pos) - Vel;
		XMStoreFloat3(&_Pos, NewPos);
	}

	if(GEngine->_Input->IsKeyHeldDn(DIK_A))
	{
		XMVECTOR Vel = DirX * DeltaSeconds * _CamMoveSpeed;
		XMVECTOR NewPos = XMLoadFloat3(&_Pos) + Vel;
		XMStoreFloat3(&_Pos, NewPos);
	}
	else if(GEngine->_Input->IsKeyHeldDn(DIK_D))
	{
		XMVECTOR Vel = DirX* DeltaSeconds * _CamMoveSpeed;
		XMVECTOR NewPos = XMLoadFloat3(&_Pos) - Vel;
		XMStoreFloat3(&_Pos, NewPos);
	}
	

	//cout_debug("x, y : %d, %d\n", GEngine->_Input->m_lDX, GEngine->_Input->m_lDY);

}

void FpsCamera::CalcViewInfo( XMMATRIX& ViewMat, XMMATRIX& ProjectionMat, float Width, float Height )
{
	XMMATRIX CamMat = XMMatrixRotationX(_Pitch) * XMMatrixRotationY(_Yaw)  * XMMatrixTranslationFromVector(XMLoadFloat3(&_Pos));
	XMVECTOR Det;
	ViewMat = XMMatrixInverse(&Det, CamMat);

	ProjectionMat = XMMatrixPerspectiveFovRH( XM_PIDIV2, Width / Height, _Near, _Far );

}
