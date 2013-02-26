#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION		0x0800
#endif
#include <dinput.h>
#include <list>

//
// ���� Receiver �鿡 ���õ� ����
//
// 1. CDInputKeyEventReceiver �� dinput�� key event�� �޴� reciever
//    ==> CInput::GetInstance()->Update() �� ȣ��ǰ� �־�� �Ѵ�.
// 2. CKeyboardEventReceiver �� window key message�� �޴� reciever
//    ==> Window MsgProc ���� CKeyboardEventGenerator::GetInstance()->OnKeyDown((int)wParam) ���·� ���� �ҷ��ش�
//        ȣ�� ���� ����.
// 3. CMouseEventReceiver �� dinput�� mouse event�� �޴� reciever
//    ==> CInput::GetInstance()->Update() �� ȣ��ǰ� �־�� �Ѵ�.
//        ����, dinput�� ������� �ʰ� 2��ó�� Window MsgProc���� ���� ȣ�����൵ �ȴ�.
//              ����, �̶��� ȣ�� ������ �����ؾ� �Ѵ�.
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
	LPDIRECTINPUT8		m_pDI;		// Direct Input ������Ʈ.
	LPDIRECTINPUTDEVICE8	m_pMouse;	// Direct Input Device(Mouse).
	LPDIRECTINPUTDEVICE8	m_pKeyboard;	// Direct Input Device(Keyboard).
	HWND			m_hWnd;		// ������ Handle

	// Mouse ����.
	DWORD	m_dwDoubleClickTime;		// ���� Ŭ�� �ð�.
	DWORD	m_dwBtn0LastClickTime;		// ��ư0�� ���������� Ŭ���� �ð�.
	DWORD	m_dwBtn1LastClickTime;		// ��ư1�� ���������� Ŭ���� �ð�.
	DWORD	m_dwBtn2LastClickTime;		// ��ư2�� ���������� Ŭ���� �ð�.
	BOOL	m_bFormerBtn0Dn;		// ������ ��ư0�� Down�̾���?
	BOOL	m_bFormerBtn1Dn;		// ������ ��ư1�� Down�̾���?
	BOOL	m_bFormerBtn2Dn;		// ������ ��ư2�� Down�̾���?

	float	m_fSensitivity;			// Mouse ����(Mouse ������ �ӵ�).
	BOOL	m_bLeftHand;			// �޼������ΰ�?

	std::list<CDInputKeyEventReceiver*>	m_lstReceiver;

	bool m_bDisable;

public:
	long	m_lScreenWidth;			// ��ũ��(BackBuffer) ���� ũ��.
	long	m_lScreenHeight;		// ��ũ��(BackBuffer) ���� ũ��.

	// Mouse ����.
	long	m_lXCoord;			// X��ǥ.
	long	m_lYCoord;			// Y��ǥ.
	long	m_lDX;				// X�� ��ȭ��.
	long	m_lDY;				// Y�� ��ȭ��.
	long	m_lDZ;				// Z�� ��ȭ��(mouse�� ��).
	BOOL	m_bLBtnDn;			// ���� ��ư Down.
	BOOL	m_bLBtnHeldDn;			// ���� ��ư Down ����.
	BOOL	m_bLBtnUp;			// ���� ��ư Up.
	BOOL	m_bLBtnDbl;			// ���� ��ư ���� Ŭ��.
	BOOL	m_bRBtnDn;			// ������ ��ư Down.
	BOOL	m_bRBtnHeldDn;			// ������ ��ư Down ����.
	BOOL	m_bRBtnUp;			// ������ ��ư Up.
	BOOL	m_bRBtnDbl;			// ������ ��ư ���� Ŭ��.
	BOOL	m_bMBtnDn;			// ���(��) ��ư Down.
	BOOL	m_bMBtnHeldDn;			// ���(��) ��ư Down ����.
	BOOL	m_bMBtnUp;			// ���(��) ��ư Up.
	BOOL	m_bMBtnDbl;			// ���(��) ��ư ���� Ŭ��.

	// Keyboard ����.
	bool	m_bKeyUp[256];			// Key Up.
	bool	m_bKeyDn[256];			// Key Down.
	bool	m_bKeyHeldDn[256];		// Key Down ����.
	bool	m_bKeyDlkDn[256];		// ����Ŭ�� ��
	bool	m_bKeyDlkHeldDn[256];		// ����Ŭ�� ��ٿ�
	float	m_fKeyLastDnTime[256];		// ���� �ֱٿ� Dn�� �߾��� ��.
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
