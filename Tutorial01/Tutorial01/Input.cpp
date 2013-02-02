//#include "Debug.h"

//#include "MouseEventGenerator.h"
//#include "KeyboardEventGenerator.h"
#include "Input.h"
#include <math.h>
#include "OutputDebug.h"
#include "CrtDbg.h"

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace std;
// val�� ���� l, �ְ� h�� ����.
#define LIMIT(val, l, h)	((val) < (l) ? (l) : (val) > (h) ? (h) : (val))

// Ŀ���� ��ġ�� Window Ŀ���� ��ġ�� ���� ������Ŵ (����׿�)
#define USE_GET_CURSOR_POS
#define CHECK_FOCUS

// Direct Input Device�� �Ҿ��� ���(Losted) �ٽ� ��� ���� �õ��� Ƚ��
#define DINPUT_NUM_REACQUIRE_TRY	8

// Mouse ����. (Default)
// SetCooperativeLevel() �Լ� Flag. ���� ����̸� Foreground���� �۵�.
#define MOUSE_COOP_FLAGS		(DISCL_NONEXCLUSIVE|DISCL_BACKGROUND)
#define MOUSE_BUFFER_SIZE		32		// ������ ũ��.

// keyboard ����. (Default)
// SetCooperativeLevel() �Լ� Flag. ���� ����̸� Foreground���� �۵�.
#define KEYBOARD_COOP_FLAGS		(DISCL_NONEXCLUSIVE|DISCL_BACKGROUND)
#define KEYBOARD_BUFFER_SIZE		512		// ������ ũ��.

// ���� �ػ�
#define NORM_RES_W			800
#define NORM_RES_H			600

BOOL CALLBACK   EnumObjectsCallback1( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
BOOL CALLBACK   EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );

Input Input::s_instance;
Input::Input(void)
{
	m_pDI		= NULL;
	m_pMouse	= NULL;
	m_pKeyboard	= NULL;
	m_hWnd		= NULL;
	m_bDisable	= false;

	m_pJoyDevice1 = NULL;
	m_bJoyReady1 = false;
	m_pJoyDiEffect1 = NULL;
	m_nForceFeedbackAxis1 = 0;
	m_bForceFeedbackEffectReady1 = false;
	m_bForceFeedbackEffectUse1 = false;
	m_procKeyInputHooker = NULL;
}

Input::~Input(void)
{
	Destroy();
}

HRESULT Input::Destroy()
{
	Release();
	m_lstReceiver.clear();

	return S_OK;
}

// Direct Input ������Ʈ �� Device ����. ��Ÿ �ʱ� ����
// hWnd		:������ �ڵ�
// lScreenWidth	: ��ũ��(Back Buffer) �ʺ�
// lScreenHeight: ��ũ��(Back Buffer) ����
// fSensitivity	: Mouse ����.(�⺻�� 1.0f��)
// bLeftHand	: �޼����� �ΰ�?(�⺻�� FALSE)
HRESULT Input::Create(HWND hWnd, long lScreenWidth, long lScreenHeight, DWORD dwMouseFlags, DWORD dwKeyboardFlags, float fSensitivity, BOOL bLeftHand)
{
	if (NULL == hWnd) return E_FAIL;
	m_hWnd = hWnd;

	if (dwMouseFlags == 0) dwMouseFlags = MOUSE_COOP_FLAGS;
	if (dwKeyboardFlags == 0) dwKeyboardFlags = KEYBOARD_COOP_FLAGS;

	HRESULT hr;

	// Direct Input ������Ʈ(m_pDI) ����.
	if (FAILED(hr = DirectInput8Create(::GetModuleHandle(NULL),
					DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDI, NULL)))
		return hr;

	// ------------------------------------------------------------------
	// Mouse ����.
	// ------------------------------------------------------------------

	// Direct Input Device(m_pMouse) ����.
	if (FAILED(hr = m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, NULL)))
	{
		Release();
		return hr;
	}

	// Mouse�� �����ϱ� ����.
	if (FAILED(hr = m_pMouse->SetDataFormat(&c_dfDIMouse)))
	{
		Release();
		return hr;
	}

	// ����, ���� �� Foreground, Background ��� ����.
	// ���⼭�� ������ define�� MOUSE_COOP_FLAGS�� ����.
	if (FAILED(hr = m_pMouse->SetCooperativeLevel(hWnd, dwMouseFlags)))
	{
		Release();
		return hr;
	}

	// Mouse Data�� ���ۿ��� ���� ���� ���� �۾�.
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize		= sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize		= sizeof(DIPROPHEADER);
	dipdw.diph.dwObj		= 0;
	dipdw.diph.dwHow		= DIPH_DEVICE;
	dipdw.dwData			= MOUSE_BUFFER_SIZE;

	if (FAILED(hr = m_pMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		Release();
		return hr;
	}

	// Mouse Device�� ȹ��.	
	int nTry;
	BOOL bAcquired = FALSE;
	for (nTry = 0; nTry < DINPUT_NUM_REACQUIRE_TRY; nTry++)
	{
		hr = m_pMouse->Acquire();
		if (hr == DI_OK || hr == S_FALSE) { bAcquired = TRUE; break; }
	}

	if (bAcquired == FALSE)
	{
		Release();
		return hr;
	}

	// ��ũ��(Back Buffer) ũ�� ����.
	m_lScreenWidth = lScreenWidth;
	m_lScreenHeight = lScreenHeight;

	// X, Y��ǥ �ʱ�ȭ.
	m_lXCoord = m_lScreenWidth / 2;		// X��ǥ�� ���� ��ġ ����.
	m_lYCoord = m_lScreenHeight / 2;	// Y��ǥ�� ���� ��ġ ����.

	m_fSensitivity = fSensitivity;		// Mouse ����(Mouse ������ �ӵ�) ����.
	m_bLeftHand = bLeftHand;		// �޼����� ���� ����.

	m_dwDoubleClickTime = ::GetDoubleClickTime();	// ���� Ŭ�� �ð��� ����.
	// �� ��ư�� ���������� Ŭ���� �ð� �ʱ�ȭ.
	m_dwBtn0LastClickTime = m_dwBtn1LastClickTime = m_dwBtn2LastClickTime = 0;
	// ������ Down���� ���°� �ʱ�ȭ.
	m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = FALSE;

	// ------------------------------------------------------------------
	// Keyboard ����.
	// ------------------------------------------------------------------

	// Direct Input Device(m_pKeyboard) ����.
	if (FAILED(hr = m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL)))
	{
		Release();
		return hr;
	}

	// Keyboard�� �����ϱ� ����.
	if (FAILED(hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		Release();
		return hr;
	}

	// ����, ���� �� Foreground, Background ��� ����.
	// ���⼭�� ������ define�� KEYBOARD_COOP_FLAGS�� ����.
	if (FAILED(hr = m_pKeyboard->SetCooperativeLevel(hWnd, dwKeyboardFlags)))
	{
		Release();
		return hr;
	}

	// Keyboard Data�� ���ۿ��� ���� ���� ���� �۾�.
	dipdw.dwData = KEYBOARD_BUFFER_SIZE;

	if (FAILED(hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		Release();
		return hr;
	}

	// Keyboard Device�� ȹ��.
	for (nTry = 0; nTry < DINPUT_NUM_REACQUIRE_TRY; nTry++)
	{
		hr = m_pKeyboard->Acquire();
		if (hr == DI_OK || hr == S_FALSE) { bAcquired = TRUE; break; }
	}

	if (bAcquired == FALSE)
	{
		Release();
		return hr;
	}

	::ZeroMemory(m_bKeyUp, sizeof(m_bKeyUp));
	::ZeroMemory(m_bKeyDn, sizeof(m_bKeyDn));
	::ZeroMemory(m_bKeyHeldDn, sizeof(m_bKeyHeldDn));

	::ZeroMemory(m_bKeyDlkDn, sizeof(m_bKeyDlkDn));
	::ZeroMemory(m_bKeyDlkHeldDn, sizeof(m_bKeyDlkHeldDn));
	::ZeroMemory(m_fKeyLastDnTime, sizeof(m_fKeyLastDnTime));

	// ------------------------------------------------------------------
	// Joystic ����.
	// ------------------------------------------------------------------
	// Joystick�� ���, ���а� ������ Release()�����ʴ´�(mouse, keyboard�� ����). �Ȼ����, �׳� �Ȼ����.

	if(FAILED(hr = m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY)))
		return hr;

	m_bJoyReady1 = true;
	if(NULL == m_pJoyDevice1)
	{
		m_bJoyReady1 = false;
		return S_OK; // ���̽�ƽ���ٰ� �ؼ� �����ʿ����.
	}

	if(FAILED(hr = m_pJoyDevice1->SetDataFormat(&c_dfDIJoystick2)))
	{
		m_pJoyDevice1 = NULL;
		m_bJoyReady1 = false;
		return hr;
	}
	if(FAILED(hr = m_pJoyDevice1->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
	{
		m_pJoyDevice1 = NULL;
		m_bJoyReady1 = false;
		return hr;
	}
	if(FAILED(hr = m_pJoyDevice1->EnumObjects(EnumObjectsCallback1, (void*)hWnd, DIDFT_ALL)))
	{
		m_pJoyDevice1 = NULL;
		m_bJoyReady1 = false;
		return hr;
	}
	m_pJoyDevice1->Acquire();
	//  �����ǵ庤 �������� �� �� �̻� �߰��Ѵٸ� �����ǵ�� ����Ʈ�� �����
	if(m_nForceFeedbackAxis1 >= 1)
	{
		if (m_nForceFeedbackAxis1 > 2) // �����ǵ�� ���� 2���� ���
			m_nForceFeedbackAxis1 = 2;
		CreateForcefeedback();
	}

	::ZeroMemory(m_bJoy1BtnUp, sizeof(m_bJoy1BtnUp));
	::ZeroMemory(m_bJoy1BtnDn, sizeof(m_bJoy1BtnDn));
	::ZeroMemory(m_bJoy1BtnHeldDn, sizeof(m_bJoy1BtnHeldDn));

	return S_OK;
}


// Direct Input ������Ʈ �� Device ����.
void Input::Release()
{
	if (m_hWnd)
	{
		m_hWnd = NULL;
	}

	if (m_pMouse)
	{
		m_pMouse->Unacquire();
		m_pMouse->Release();
		m_pMouse = NULL;
	}

	if (m_pKeyboard)
	{
		m_pKeyboard->Unacquire();
		m_pKeyboard->Release();
		m_pKeyboard = NULL;
	}

	if (m_pDI)
	{
		m_pDI->Release();
		m_pDI = NULL;
	}

	if (m_pJoyDevice1) 
	{
		m_pJoyDevice1->Unacquire();
		m_pJoyDevice1->Release();
		m_pJoyDevice1 = NULL;
		m_bJoyReady1 = false;
	}

	if (m_pJoyDiEffect1)
	{
		m_pJoyDiEffect1->Release();
		m_pJoyDiEffect1 = NULL;
	}
}


// �Լ� ���� : ���콺 ��ư �� �ʱ�ȭ.(Held�� ����)
void Input::Reset()
{
	m_bLBtnUp = m_bRBtnUp = m_bMBtnUp
		= m_bLBtnDn = m_bRBtnDn = m_bMBtnDn
		= m_bLBtnDbl = m_bRBtnDbl = m_bMBtnDbl = FALSE;
}

// Update
int Input::Update()
{
	if (NULL == m_pMouse || NULL == m_pKeyboard) return -1;

#ifdef CHECK_FOCUS
	// ���ӿ� Focus�� �� ���� �ʴٸ� DirectInput�� ó������ �ʴ´�.
	if (m_hWnd != GetForegroundWindow()) return 0;
#endif // CHECK_FOCUS

	HRESULT			hr;
	// Mouse Data�� ���� ����.
	DIDEVICEOBJECTDATA	didodM[MOUSE_BUFFER_SIZE];
	// Keyboard Data�� ���� ����.
	DIDEVICEOBJECTDATA	didodK[KEYBOARD_BUFFER_SIZE];
	DWORD			dwElements;

	// ------------------------------------------------------------------
	// Mouse ����.
	// ------------------------------------------------------------------

	dwElements = MOUSE_BUFFER_SIZE;
	// Mouse�� Data�� ���� ����(didodM[])�� ����.
	hr = m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didodM, &dwElements, 0);

	if (hr != DI_OK)	// hr�� DI_BUFFEROVERFLOW�̰ų� �����ڵ��̸�.
	{
		//OutputDebugString("Mouse Input Device Losted.\n");

		// Mouse Device�� �ٽ� ȹ��.	
		int nTry;
		BOOL bAcquired = FALSE;
		for (nTry = 0; nTry < DINPUT_NUM_REACQUIRE_TRY; nTry++)
		{
			hr = m_pMouse->Acquire();
			if (hr == DI_OK || hr == S_FALSE) { bAcquired = TRUE;break; }
		}

		if (bAcquired == FALSE)
		{
			return -1;
		}

		cout_debug("Mouse Input Device Reacqired.\n");

		hr = m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didodM, &dwElements, 0);
		if (hr != DI_OK) return -1;
	}

	// �� ���� ��ȭ�� �ʱ�ȭ.
	m_lDX = m_lDY = m_lDZ = 0L;
	// Down, Up, Double Click�� �������� TRUE�� �ǾߵǹǷ� FALSE�� �ʱ�ȭ ��.
	Reset();

	// ������ Data ó��.
	for (DWORD i = 0; i < dwElements; i++)
	{
		switch (didodM[i].dwOfs)
		{
			// �� ���� ��ȭ�� Data�� ���.
			case DIMOFS_X:
				m_lDX += long((long)didodM[i].dwData * m_fSensitivity);
				break;

			case DIMOFS_Y:
				m_lDY += long((long)didodM[i].dwData * m_fSensitivity);
				break;

			case DIMOFS_Z:
				m_lDZ += long((long)didodM[i].dwData * m_fSensitivity);
				break;

				// �� ��ư ó��.
			case DIMOFS_BUTTON0:
				if (didodM[i].dwData & 0x80)// ��ư0�� ���ȴٸ�.
				{
					// ���� Ŭ���̰� ������ �� Ŭ���̾���.
					// 'm_bFormerBtn1Dn'�� ��ư�� ��Ÿ���� �� Dbl�� �����
					// �������ϱ� ���ؼ�.
					if (didodM[i].dwTimeStamp - m_dwBtn0LastClickTime
							<= m_dwDoubleClickTime && m_bFormerBtn0Dn)
					{
						if (m_bLeftHand) // �޼����̸�.
						{
							m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = TRUE;
							m_bRBtnUp = FALSE;
						}
						else // ���������̸�.
						{
							m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = TRUE;
							m_bLBtnUp = FALSE;
						}
						m_bFormerBtn0Dn = FALSE;
					}
					else // �� Ŭ���̸�.
					{
						if (m_bLeftHand) // �޼����̸�.
						{
							m_bRBtnDn = m_bRBtnHeldDn = TRUE;
							m_bRBtnUp = m_bRBtnDbl = FALSE;
						}
						else // ���������̸�.
						{
							m_bLBtnDn = m_bLBtnHeldDn = TRUE;
							m_bLBtnUp = m_bLBtnDbl = FALSE;
						}
						m_bFormerBtn0Dn = TRUE;
					}
					m_dwBtn0LastClickTime = didodM[i].dwTimeStamp;
				}
				else // ��ư0�� �ö�Դٸ�.
				{
					if (m_bLeftHand) // �޼����̸�.
					{
						m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = FALSE;
						m_bRBtnUp = TRUE;
					}
					else // ���������̸�.
					{
						m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = FALSE;
						m_bLBtnUp = TRUE;
					}
				}
				break;

			case DIMOFS_BUTTON1:
				if (didodM[i].dwData & 0x80)// ��ư1�� ���ȴٸ�.
				{
					// ���� Ŭ���̰� ������ �� Ŭ���̾���.
					// 'm_bFormerBtn1Dn'�� ��ư�� ��Ÿ���� �� Dbl�� �����
					// �������ϱ� ���ؼ�.
					if (didodM[i].dwTimeStamp - m_dwBtn1LastClickTime
							<= m_dwDoubleClickTime && m_bFormerBtn1Dn)
					{
						if (m_bLeftHand) // �޼����̸�.
						{
							m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = TRUE;
							m_bLBtnUp = FALSE;
						}
						else // ���������̸�.
						{
							m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = TRUE;
							m_bRBtnUp = FALSE;
						}
						m_bFormerBtn1Dn = FALSE;
					}
					else // �� Ŭ���̸�.
					{
						if (m_bLeftHand) // �޼����̸�.
						{
							m_bLBtnDn = m_bLBtnHeldDn = TRUE;
							m_bLBtnUp = m_bLBtnDbl = FALSE;
						}
						else // ���������̸�.
						{
							m_bRBtnDn = m_bRBtnHeldDn = TRUE;
							m_bRBtnUp = m_bRBtnDbl = FALSE;
						}
						m_bFormerBtn1Dn = TRUE;
					}
					m_dwBtn1LastClickTime = didodM[i].dwTimeStamp;
				}
				else // ��ư1�� �ö�Դٸ�.
				{
					if (m_bLeftHand) // �޼����̸�.
					{
						m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = FALSE;
						m_bLBtnUp = TRUE;
					}
					else // ���������̸�.
					{
						m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = FALSE;
						m_bRBtnUp = TRUE;
					}
				}
				break;

			case DIMOFS_BUTTON2: // �� ��ư.
				if (didodM[i].dwData & 0x80) // ��ư2�� ���ȴٸ�.
				{
					// ���� Ŭ���̰� ������ �� Ŭ���̾���.
					// 'm_bFormerBtn2Dn'�� ��ư�� ��Ÿ���� �� Dbl�� �����
					// �������ϱ� ���ؼ�.
					if (didodM[i].dwTimeStamp - m_dwBtn2LastClickTime
							<= m_dwDoubleClickTime && m_bFormerBtn2Dn)
					{
						m_bMBtnDbl = TRUE;
						m_bFormerBtn2Dn = FALSE;
					}
					else // �� Ŭ���̸�.
					{
						m_bFormerBtn2Dn = TRUE;
						m_bMBtnDbl = FALSE;
					}
					m_bMBtnDn = m_bMBtnHeldDn = TRUE;
					m_bMBtnUp = FALSE;
					m_dwBtn2LastClickTime = didodM[i].dwTimeStamp;
				}
				else // ��ư2�� �ö�Դٸ�.
				{
					m_bMBtnDn = m_bMBtnHeldDn = m_bMBtnDbl = FALSE;
					m_bMBtnUp = TRUE;
				}
				break;
		}	// switch��.
	}	// for��.

	DIMOUSESTATE mouseState;
	m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState);
	if(mouseState.rgbButtons[0] & 0x80)
		m_bLBtnHeldDn = TRUE;
	else
		m_bLBtnHeldDn = FALSE;

	if(mouseState.rgbButtons[1] & 0x80)
		m_bRBtnHeldDn = TRUE;
	else
		m_bRBtnHeldDn = FALSE;

	if(mouseState.rgbButtons[2] & 0x80)
		m_bMBtnHeldDn = TRUE;
	else
		m_bMBtnHeldDn = FALSE;


	// X, Y���� ��ȭ���� ��ǥ�� ����.
	long lPrevXCoord = m_lXCoord;
	long lPrevYCoord = m_lYCoord;

#ifdef USE_GET_CURSOR_POS
	POINT curPos;
	GetCursorPos(&curPos);
	ScreenToClient(m_hWnd, &curPos);
	m_lXCoord = curPos.x;
	m_lYCoord = curPos.y;
#else
	m_lXCoord += m_lDX;
	m_lYCoord += m_lDY;
#endif

	// X, Y��ǥ���� ������.
	m_lXCoord = LIMIT(m_lXCoord, 0, m_lScreenWidth - 1);
	m_lYCoord = LIMIT(m_lYCoord, 0, m_lScreenHeight - 1);

	// ���� resolution ���� normalize �Ѵ�
	m_lXCoord = (long)((float)m_lXCoord/(float)m_lScreenWidth*(float)NORM_RES_W);
	m_lYCoord = (long)((float)m_lYCoord/(float)m_lScreenHeight*(float)NORM_RES_H);

	// ------------------------------------------------------------------
	// Keyboard ����.
	// ------------------------------------------------------------------

	// Up�� Dn�� �����Ǵ� ��찡 �����Ƿ� �ʱ�ȭ.
	// HeldDown�� ������ �ִ� ����(Up�� �Ǳ� ������) �����ǹǷ� �ʱ�ȭ�� ����.
	::ZeroMemory(m_bKeyUp, sizeof(m_bKeyUp));
	::ZeroMemory(m_bKeyDn, sizeof(m_bKeyDn));
	::ZeroMemory(m_bKeyDlkDn, sizeof(m_bKeyDlkDn));

	dwElements = KEYBOARD_BUFFER_SIZE;
	// Keyboard�� Data�� ���� ����(didodK[])�� ����.
	hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didodK, &dwElements, 0 );

	if (hr != DI_OK) // hr�� DI_BUFFEROVERFLOW�̰ų� �����ڵ��̸�.
	{
		//OutputDebugString("Keyboard Input Device Losted.\n");

		// Mouse Device�� �ٽ� ȹ��.	
		int nTry;
		BOOL bAcquired = FALSE;
		for (nTry = 0; nTry < DINPUT_NUM_REACQUIRE_TRY; nTry++)
		{
			hr = m_pKeyboard->Acquire();
			if (hr == DI_OK || hr == S_FALSE) { bAcquired = TRUE;break; }
		}

		if (bAcquired == FALSE) return -1;

		cout_debug("Keyboard Input Device Reacqired.\n");

		hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didodM, &dwElements, 0);
		if (hr != DI_OK) return -1;
	}


	// ������ Data ó��.
	for (int i = 0; i < (int)dwElements; i++)
	{
		if (didodK[i].dwData & 0x80)	// Key Down�̸�.
		{
			m_bKeyHeldDn[didodK[i].dwOfs] = true;
			m_bKeyDn[didodK[i].dwOfs] = true;
			m_bKeyUp[didodK[i].dwOfs] = false;

			if (didodK[i].dwTimeStamp - m_fKeyLastDnTime[didodK[i].dwOfs] <= 200)
			{
				::ZeroMemory(m_fKeyLastDnTime, sizeof(m_fKeyLastDnTime));
				m_bKeyDlkDn[didodK[i].dwOfs] = true;
				m_bKeyDlkHeldDn[didodK[i].dwOfs] = true;
			}
			else
			{
				// ����Ŭ���� �Ѽ����� �ϳ��� �����ϵ��� �Ѵ�
				::ZeroMemory(m_fKeyLastDnTime, sizeof(m_fKeyLastDnTime));
				m_fKeyLastDnTime[didodK[i].dwOfs] = (float)didodK[i].dwTimeStamp;
			}

			if (m_procKeyInputHooker) m_procKeyInputHooker(didodK[i].dwOfs, didodK[i].dwTimeStamp, true);
		}
		else	// Key Up�̸�.
		{
			m_bKeyHeldDn[didodK[i].dwOfs] = false;
			m_bKeyDn[didodK[i].dwOfs] = false;
			m_bKeyUp[didodK[i].dwOfs] = true;
			m_bKeyDlkDn[didodK[i].dwOfs] = false;
			m_bKeyDlkHeldDn[didodK[i].dwOfs] = false;

			if (m_procKeyInputHooker) m_procKeyInputHooker(didodK[i].dwOfs, didodK[i].dwTimeStamp, false);
		}
	}

	// �ش� Mouse Event�� ���� ó���� �Ѵ�.

	// TODO: ����Ŭ���� ����
	// �����غ���. ����Ŭ���� �� ������Ʈ�� �˻��ϴ� ���� ���� �� ���⵵ �ϴ�. �ֳ��ϸ� ���⼭ �˻��ϴ� �����δ�
	// ���� ������Ʈ�� ���� Ŭ���ߴ��� �� �� ���� �����̴�.	

	//if (m_bLBtnHeldDn) CMouseEventGenerator::GetInstance()->OnLButtonHeldDown((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bLBtnUp) CMouseEventGenerator::GetInstance()->OnLButtonUp((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bLBtnDn) CMouseEventGenerator::GetInstance()->OnLButtonDown((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bLBtnDbl) CMouseEventGenerator::GetInstance()->OnLButtonDblClk((int)m_lXCoord, (int)m_lYCoord);

	//if (m_bRBtnHeldDn) CMouseEventGenerator::GetInstance()->OnRButtonHeldDown((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bRBtnUp) CMouseEventGenerator::GetInstance()->OnRButtonUp((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bRBtnDn) CMouseEventGenerator::GetInstance()->OnRButtonDown((int)m_lXCoord, (int)m_lYCoord);

	//if (m_bMBtnHeldDn) CMouseEventGenerator::GetInstance()->OnMButtonHeldDown((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bMBtnUp) CMouseEventGenerator::GetInstance()->OnMButtonUp((int)m_lXCoord, (int)m_lYCoord);
	//if (m_bMBtnDn) CMouseEventGenerator::GetInstance()->OnMButtonDown((int)m_lXCoord, (int)m_lYCoord);

	//if ((m_lXCoord - lPrevXCoord) != 0 || (m_lYCoord - lPrevYCoord) != 0) // �̵��� ��ȭ�� �ִٸ�
	//	CMouseEventGenerator::GetInstance()->OnMouseMove((int)m_lXCoord, (int)m_lYCoord);

	//if (m_lDZ) // ��ó��
	//	CMouseEventGenerator::GetInstance()->OnMouseWheel((int)m_lXCoord, (int)m_lYCoord, m_lDZ);

	// NOTE : Mouse�� ���, DirectInput�� Window Message�� Receiver�� ������,
	// Keyboard�� ���� Receiver�� �ٸ���. �̷��� �ٸ��� ó���ϴ� ������, VK �� DIK code�� ���� �ٸ���,
	// IME�� ó���ϴµ� �־ ������� �ʱ� �����̴�. ���� Keyboard�� ��� Receiver�� �ٸ��� �Ѵ�.

	// �ش� Key Event�� ���� ó���� �Ѵ�.
	/*std::list<CDInputKeyEventReceiver*>::iterator inputIter;
	for (int i = 0; i < 256; i++)
	{
		if (m_bKeyHeldDn[i])
		{
			for(inputIter = m_lstReceiver.begin(); inputIter != m_lstReceiver.end(); inputIter++)
			{
				(*inputIter)->OnKeyHeldDown(i);
			}
		}

		if (m_bKeyDn[i])
		{
			for(inputIter = m_lstReceiver.begin(); inputIter != m_lstReceiver.end(); inputIter++)
			{
				(*inputIter)->OnKeyDown(i);
			}
		}

		if (m_bKeyUp[i])
		{
			std::list<CDInputKeyEventReceiver*>::iterator inputIter;
			for(inputIter = m_lstReceiver.begin(); inputIter != m_lstReceiver.end(); inputIter++)
			{
				(*inputIter)->OnKeyUp(i);
			}
		}
	}*/


	// ------------------------------------------------------------------
	// Joystic ����.
	// ------------------------------------------------------------------
	const int JOY_AXIS_THRESHOLD = 500;

	::ZeroMemory(m_bJoy1BtnUp, sizeof(m_bJoy1BtnUp));
	::ZeroMemory(m_bJoy1BtnDn, sizeof(m_bJoy1BtnDn));
	::ZeroMemory(m_lJoyAxis, sizeof(m_lJoyAxis));
	::ZeroMemory(m_rJoyAxis, sizeof(m_rJoyAxis));

	if (m_bJoyReady1 == true)
	{
		// poll
		hr = m_pJoyDevice1->Poll(); 
		if (FAILED(hr))  
		{
			// Reacqure
			int nTry;
			BOOL bAcquired = FALSE;
			for (nTry = 0; nTry < DINPUT_NUM_REACQUIRE_TRY; nTry++)
			{
				hr = m_pJoyDevice1->Acquire();
				if (hr == DI_OK || hr == S_FALSE) { bAcquired = TRUE;break; }
			}
			if (bAcquired == FALSE) return -1;
			cout_debug("Joystick Input Device Reacqired.\n");
		}

		DIJOYSTATE2 js;
		hr = m_pJoyDevice1->GetDeviceState(sizeof(DIJOYSTATE2), &js);
		if (hr == DI_OK)
		{
			m_lJoyAxis[0] = js.lX;
			m_lJoyAxis[1] = js.lY;
			m_lJoyAxis[2] = js.lZ;

			m_rJoyAxis[0] = js.lRx;
			m_rJoyAxis[1] = js.lRy;
			m_rJoyAxis[2] = js.lRz;
		}

		//cout_debug("%ld %ld %d %ld %ld %d\n", m_lJoyAxis[0], m_lJoyAxis[1], m_lJoyAxis[2],
		//	m_rJoyAxis[0], m_rJoyAxis[1], m_rJoyAxis[2]);

		for(int i=0; i<128; ++i)
		{
			if (js.rgbButtons[i] & 0x80) // �ٿ�
			{
				if (m_bJoy1BtnHeldDn[i] == true) // �������� �ٿ�
					m_bJoy1BtnDn[i] = false;
				else
					m_bJoy1BtnDn[i] = true;
				m_bJoy1BtnUp[i] = false;
				m_bJoy1BtnHeldDn[i] = true;
			}
			else // ��
			{
				if (m_bJoy1BtnHeldDn[i] == false) // �������� ��
					m_bJoy1BtnUp[i] = false;
				else
					m_bJoy1BtnUp[i] = true;
				m_bJoy1BtnDn[i] = false;
				m_bJoy1BtnHeldDn[i] = false;
			}
		}

		//cout_debug("%d %d %d\n", m_bJoy1BtnUp[5], m_bJoy1BtnDn[5], m_bJoy1BtnHeldDn[5]);
	}

	return 0;
}

void Input::SubReceiver(CDInputKeyEventReceiver* pReceiver)
{
	list<CDInputKeyEventReceiver*>::iterator i;
	for(i=m_lstReceiver.begin();i!=m_lstReceiver.end();i++)
	{
		CDInputKeyEventReceiver* pItem = *i;
		if(pItem == pReceiver)
		{
			m_lstReceiver.erase(i);
			return;
		}
	}
}

bool Input::IsKeyUp(int nKey)
{
	if(m_bDisable)
		return false;
	else
		return m_bKeyUp[nKey];
}

bool Input::IsKeyDn(int nKey)
{
	if(m_bDisable)
		return false;
	else
		return m_bKeyDn[nKey];
}

bool Input::IsKeyHeldDn(int nKey)
{
	if(m_bDisable)
		return false;
	else
		return m_bKeyHeldDn[nKey];
}

bool Input::IsKeyDIkDn(int nKey)
{
	if(m_bDisable)
		return false;
	else
		return m_bKeyDlkDn[nKey];
}

bool Input::IsKeyDlkHeldDn(int nKey)
{
	if(m_bDisable)
		return false;
	else
		return m_bKeyDlkHeldDn[nKey];
}

void Input::CreateForcefeedback()
{
	HRESULT hr = S_OK;

	DWORD           rgdwAxes[2]     = { DIJOFS_X, DIJOFS_Y };
	LONG            rglDirection[2] = { 0, 0 };
	DICONSTANTFORCE cf              = { 0 };

	DIEFFECT eff;
	ZeroMemory( &eff, sizeof(eff) );
	eff.dwSize                  = sizeof(DIEFFECT);
	eff.dwFlags                 = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.dwDuration              = (DWORD)(300000); // ����Ʈ ���� �ð�
	eff.dwSamplePeriod          = 0;
	eff.dwGain                  = DI_FFNOMINALMAX;
	eff.dwTriggerButton         = DIEB_NOTRIGGER;
	eff.dwTriggerRepeatInterval = 0;
	eff.cAxes                   = m_nForceFeedbackAxis1;
	eff.rgdwAxes                = rgdwAxes;
	eff.rglDirection            = rglDirection;
	eff.lpEnvelope              = 0;
	eff.cbTypeSpecificParams    = sizeof(DICONSTANTFORCE);
	eff.lpvTypeSpecificParams   = &cf;
	eff.dwStartDelay            = 0;

	if(FAILED( hr = m_pJoyDevice1->CreateEffect(GUID_ConstantForce, &eff, &m_pJoyDiEffect1, NULL)))
	{
		m_bForceFeedbackEffectReady1 = false;
		return;
	}
	m_bForceFeedbackEffectReady1 = true;
	return;
}

void Input::OnForcefeedback(int nXForce, int nYForce)
{
	if (!m_bForceFeedbackEffectReady1 || !m_bForceFeedbackEffectUse1)
		return;

	LONG rglDirection[2] = { 0, 0 };

	DICONSTANTFORCE cf;
	if(m_nForceFeedbackAxis1 == 1)
	{
		cf.lMagnitude = nXForce;
		rglDirection[0] = 0;
	}
	else
	{
		rglDirection[0] = nXForce;
		rglDirection[1] = nYForce;
		cf.lMagnitude = (DWORD)sqrt((double)nXForce * (double)nXForce + (double)nYForce * (double)nYForce);
	}
	DIEFFECT eff;
	ZeroMemory( &eff, sizeof(eff) );
	eff.dwSize                = sizeof(DIEFFECT);
	eff.dwFlags               = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.cAxes                 = m_nForceFeedbackAxis1 ;
	eff.rglDirection          = rglDirection;
	eff.lpEnvelope            = 0;
	eff.cbTypeSpecificParams  = sizeof(DICONSTANTFORCE);
	eff.lpvTypeSpecificParams = &cf;
	eff.dwStartDelay          = 0;

	m_pJoyDiEffect1->SetParameters( &eff, DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START );
	return;
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;
	if (Input::GetInstance()->m_pJoyDevice1 == NULL)
	{
		hr = Input::GetInstance()->m_pDI->CreateDevice(pdidInstance->guidInstance, &(Input::GetInstance()->m_pJoyDevice1), NULL);
		if(FAILED(hr)) return DIENUM_CONTINUE;
		return DIENUM_STOP;
	}
	return DIENUM_STOP;
}

BOOL CALLBACK EnumObjectsCallback1(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
	HWND hHWnd = (HWND)pContext;
	if (pdidoi->dwType & DIDFT_AXIS)
	{
		DIPROPRANGE diprg; 
		diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
		diprg.diph.dwHow        = DIPH_BYID; 
		diprg.diph.dwObj        = pdidoi->dwType; 
		diprg.lMin              = -1000; 
		diprg.lMax              = +1000; 

		if(FAILED(Input::GetInstance()->m_pJoyDevice1->SetProperty(DIPROP_RANGE, &diprg.diph))) 
			return DIENUM_STOP;         
	}
	if((pdidoi->dwFlags & DIDOI_FFACTUATOR) != 0)
		Input::GetInstance()->m_nForceFeedbackAxis1++;
	return DIENUM_CONTINUE;
}
