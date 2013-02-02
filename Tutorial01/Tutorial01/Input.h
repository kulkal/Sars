#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION		0x0800
#endif
#include <dinput.h>
#include <list>

//
// 여러 Receiver 들에 관련된 정리
//
// 1. CDInputKeyEventReceiver 는 dinput의 key event를 받는 reciever
//    ==> CInput::GetInstance()->Update() 가 호출되고 있어야 한다.
// 2. CKeyboardEventReceiver 는 window key message를 받는 reciever
//    ==> Window MsgProc 에서 CKeyboardEventGenerator::GetInstance()->OnKeyDown((int)wParam) 형태로 직접 불러준다
//        호출 시점 주의.
// 3. CMouseEventReceiver 는 dinput의 mouse event를 받는 reciever
//    ==> CInput::GetInstance()->Update() 가 호출되고 있어야 한다.
//        물론, dinput을 사용하지 않고 2번처럼 Window MsgProc에서 직접 호출해줘도 된다.
//              역시, 이때는 호출 시점을 주의해야 한다.
//

class CDInputKeyEventReceiver
{
public:
	virtual void OnKeyHeldDown(int nKey) {nKey;}
	virtual void OnKeyDown(int nKey) {nKey;}
	virtual void OnKeyUp(int nKey) {nKey;}
};

typedef void (*ProcKeyInputHooker)(DWORD dwKey, DWORD dwTimeStamp, bool bKeyDn);

class Input
{
protected:
	static Input		s_instance;
	LPDIRECTINPUT8		m_pDI;		// Direct Input 오브젝트.
	LPDIRECTINPUTDEVICE8	m_pMouse;	// Direct Input Device(Mouse).
	LPDIRECTINPUTDEVICE8	m_pKeyboard;	// Direct Input Device(Keyboard).
	HWND			m_hWnd;		// 윈도우 Handle

	// Mouse 관련.
	DWORD	m_dwDoubleClickTime;		// 더블 클릭 시간.
	DWORD	m_dwBtn0LastClickTime;		// 버튼0이 마지막으로 클릭한 시간.
	DWORD	m_dwBtn1LastClickTime;		// 버튼1이 마지막으로 클릭한 시간.
	DWORD	m_dwBtn2LastClickTime;		// 버튼2가 마지막으로 클릭한 시간.
	BOOL	m_bFormerBtn0Dn;		// 이전에 버튼0가 Down이었나?
	BOOL	m_bFormerBtn1Dn;		// 이전에 버튼1가 Down이었나?
	BOOL	m_bFormerBtn2Dn;		// 이전에 버튼2가 Down이었나?

	float	m_fSensitivity;			// Mouse 감도(Mouse 포인터 속도).
	BOOL	m_bLeftHand;			// 왼손잡이인가?

	std::list<CDInputKeyEventReceiver*>	m_lstReceiver;

	bool m_bDisable;

public:
	long	m_lScreenWidth;			// 스크린(BackBuffer) 가로 크기.
	long	m_lScreenHeight;		// 스크린(BackBuffer) 세로 크기.

	// Mouse 관련.
	long	m_lXCoord;			// X좌표.
	long	m_lYCoord;			// Y좌표.
	long	m_lDX;				// X축 변화량.
	long	m_lDY;				// Y축 변화량.
	long	m_lDZ;				// Z축 변화량(mouse의 휠).
	BOOL	m_bLBtnDn;			// 왼쪽 버튼 Down.
	BOOL	m_bLBtnHeldDn;			// 왼쪽 버튼 Down 유지.
	BOOL	m_bLBtnUp;			// 왼쪽 버튼 Up.
	BOOL	m_bLBtnDbl;			// 왼쪽 버튼 더블 클릭.
	BOOL	m_bRBtnDn;			// 오른쪽 버튼 Down.
	BOOL	m_bRBtnHeldDn;			// 오른쪽 버튼 Down 유지.
	BOOL	m_bRBtnUp;			// 오른쪽 버튼 Up.
	BOOL	m_bRBtnDbl;			// 오른쪽 버튼 더블 클릭.
	BOOL	m_bMBtnDn;			// 가운데(휠) 버튼 Down.
	BOOL	m_bMBtnHeldDn;			// 가운데(휠) 버튼 Down 유지.
	BOOL	m_bMBtnUp;			// 가운데(휠) 버튼 Up.
	BOOL	m_bMBtnDbl;			// 가운데(휠) 버튼 더블 클릭.

	// Keyboard 관련.
	bool	m_bKeyUp[256];			// Key Up.
	bool	m_bKeyDn[256];			// Key Down.
	bool	m_bKeyHeldDn[256];		// Key Down 유지.
	bool	m_bKeyDlkDn[256];		// 더블클릭 업
	bool	m_bKeyDlkHeldDn[256];		// 더블클릭 헬다운
	float	m_fKeyLastDnTime[256];		// 가장 최근에 Dn을 했었을 때.
	ProcKeyInputHooker m_procKeyInputHooker;

public:
	static Input* GetInstance() { return &s_instance; }
	HRESULT Create(HWND hWnd, long lScreenWidth, long lScreenHeight, DWORD dwMouseFlags, DWORD dwKeyboardFlags, float fSensitivity = 1.0f, BOOL bLeftHand = FALSE);
	HRESULT Destroy();
	void SetDisable(bool bDisable){m_bDisable = bDisable;}
	void SetProcKeyInputHooker(ProcKeyInputHooker procKeyInputHooker){m_procKeyInputHooker = procKeyInputHooker;}

	bool IsKeyUp(int nKey);
	bool IsKeyDn(int nKey);
	bool IsKeyHeldDn(int nKey);
	bool IsKeyDIkDn(int nKey);
	bool IsKeyDlkHeldDn(int nKey);

	void Release();
	void Reset();
	void AddReceiver(CDInputKeyEventReceiver* pReceiver) { m_lstReceiver.push_back(pReceiver); }
	void SubReceiver(CDInputKeyEventReceiver* pReceiver);
	void ClearReceiver() { m_lstReceiver.clear(); m_procKeyInputHooker = NULL; }

	int Update();

	void _CorrectInputState();
	Input();
	virtual ~Input();

public:
	HWND GetWnd(){return m_hWnd;}
	void SetForcefeedback(bool use) { m_bForceFeedbackEffectUse1 = use; }
	void OnForcefeedback(int nXForce, int nYForce);
	bool IsJoyReady() { return m_bJoyReady1; }

	LONG IsJoy1Axis(int num) { return m_lJoyAxis[num]; }
	LONG IsJoy2Axis(int num) { return m_rJoyAxis[num]; }
	
	bool IsJoy1BtnUp(int num) { return m_bJoy1BtnUp[num]; }
	bool IsJoy1BtnDn(int num) { return m_bJoy1BtnDn[num]; }
	bool IsJoy1BtnHeldDn(int num) { return m_bJoy1BtnHeldDn[num]; }

protected:
	void CreateForcefeedback();  

	LPDIRECTINPUTDEVICE8	m_pJoyDevice1;     
	bool			m_bJoyReady1;				

	LPDIRECTINPUTEFFECT	m_pJoyDiEffect1;				
	DWORD                   m_nForceFeedbackAxis1;	
	bool			m_bForceFeedbackEffectReady1;			
	bool			m_bForceFeedbackEffectUse1;			

	LONG	m_lJoyAxis[128];
	LONG	m_rJoyAxis[128];

	bool	m_bJoy1BtnUp[128];
	bool	m_bJoy1BtnDn[128];
	bool	m_bJoy1BtnHeldDn[128];

public:
	friend BOOL CALLBACK    EnumObjectsCallback1( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
	friend BOOL CALLBACK    EnumObjectsCallback2( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
	friend BOOL CALLBACK    EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
	friend BOOL CALLBACK	DIEnumEffectsProc(LPCDIEFFECTINFO pei, LPVOID pv);
};
