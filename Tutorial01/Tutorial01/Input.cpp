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
// val을 최저 l, 최고 h로 제한.
#define LIMIT(val, l, h)	((val) < (l) ? (l) : (val) > (h) ? (h) : (val))

// 커서의 위치가 Window 커서의 위치와 같게 고정시킴 (디버그용)
#define USE_GET_CURSOR_POS
#define CHECK_FOCUS

// Direct Input Device를 잃었을 경우(Losted) 다시 얻기 위해 시도할 횟수
#define DINPUT_NUM_REACQUIRE_TRY	8

// Mouse 관련. (Default)
// SetCooperativeLevel() 함수 Flag. 독점 모드이며 Foreground에서 작동.
#define MOUSE_COOP_FLAGS		(DISCL_NONEXCLUSIVE|DISCL_BACKGROUND)
#define MOUSE_BUFFER_SIZE		32		// 버퍼의 크기.

// keyboard 관련. (Default)
// SetCooperativeLevel() 함수 Flag. 비독점 모드이며 Foreground에서 작동.
#define KEYBOARD_COOP_FLAGS		(DISCL_NONEXCLUSIVE|DISCL_BACKGROUND)
#define KEYBOARD_BUFFER_SIZE		512		// 버퍼의 크기.

// 기준 해상도
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

// Direct Input 오브젝트 및 Device 생성. 기타 초기 설정
// hWnd		:윈도우 핸들
// lScreenWidth	: 스크린(Back Buffer) 너비
// lScreenHeight: 스크린(Back Buffer) 높이
// fSensitivity	: Mouse 감도.(기본값 1.0f배)
// bLeftHand	: 왼손잡이 인가?(기본값 FALSE)
HRESULT Input::Create(HWND hWnd, long lScreenWidth, long lScreenHeight, DWORD dwMouseFlags, DWORD dwKeyboardFlags, float fSensitivity, BOOL bLeftHand)
{
	if (NULL == hWnd) return E_FAIL;
	m_hWnd = hWnd;

	if (dwMouseFlags == 0) dwMouseFlags = MOUSE_COOP_FLAGS;
	if (dwKeyboardFlags == 0) dwKeyboardFlags = KEYBOARD_COOP_FLAGS;

	HRESULT hr;

	// Direct Input 오브젝트(m_pDI) 생성.
	if (FAILED(hr = DirectInput8Create(::GetModuleHandle(NULL),
					DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDI, NULL)))
		return hr;

	// ------------------------------------------------------------------
	// Mouse 관련.
	// ------------------------------------------------------------------

	// Direct Input Device(m_pMouse) 생성.
	if (FAILED(hr = m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, NULL)))
	{
		Release();
		return hr;
	}

	// Mouse를 제어하기 위해.
	if (FAILED(hr = m_pMouse->SetDataFormat(&c_dfDIMouse)))
	{
		Release();
		return hr;
	}

	// 독점, 비독점 및 Foreground, Background 모드 선택.
	// 여기서는 위에서 define된 MOUSE_COOP_FLAGS를 따름.
	if (FAILED(hr = m_pMouse->SetCooperativeLevel(hWnd, dwMouseFlags)))
	{
		Release();
		return hr;
	}

	// Mouse Data를 버퍼에서 얻어내기 위한 사전 작업.
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

	// Mouse Device를 획득.	
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

	// 스크린(Back Buffer) 크기 세팅.
	m_lScreenWidth = lScreenWidth;
	m_lScreenHeight = lScreenHeight;

	// X, Y좌표 초기화.
	m_lXCoord = m_lScreenWidth / 2;		// X좌표의 최초 위치 설정.
	m_lYCoord = m_lScreenHeight / 2;	// Y좌표의 최초 위치 설정.

	m_fSensitivity = fSensitivity;		// Mouse 감도(Mouse 포인터 속도) 세팅.
	m_bLeftHand = bLeftHand;		// 왼손잡이 여부 세팅.

	m_dwDoubleClickTime = ::GetDoubleClickTime();	// 더블 클릭 시간을 얻음.
	// 각 버튼의 마지막으로 클릭한 시간 초기화.
	m_dwBtn0LastClickTime = m_dwBtn1LastClickTime = m_dwBtn2LastClickTime = 0;
	// 이전에 Down여부 상태값 초기화.
	m_bFormerBtn0Dn = m_bFormerBtn1Dn = m_bFormerBtn2Dn = FALSE;

	// ------------------------------------------------------------------
	// Keyboard 관련.
	// ------------------------------------------------------------------

	// Direct Input Device(m_pKeyboard) 생성.
	if (FAILED(hr = m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL)))
	{
		Release();
		return hr;
	}

	// Keyboard를 제어하기 위해.
	if (FAILED(hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		Release();
		return hr;
	}

	// 독점, 비독점 및 Foreground, Background 모드 선택.
	// 여기서는 위에서 define된 KEYBOARD_COOP_FLAGS를 따름.
	if (FAILED(hr = m_pKeyboard->SetCooperativeLevel(hWnd, dwKeyboardFlags)))
	{
		Release();
		return hr;
	}

	// Keyboard Data를 버퍼에서 얻어내기 위한 사전 작업.
	dipdw.dwData = KEYBOARD_BUFFER_SIZE;

	if (FAILED(hr = m_pKeyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		Release();
		return hr;
	}

	// Keyboard Device를 획득.
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
	// Joystic 관련.
	// ------------------------------------------------------------------
	// Joystick의 경우, 실패가 나더라도 Release()하지않는다(mouse, keyboard도 날라감). 안생기면, 그냥 안생길뿐.

	if(FAILED(hr = m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, NULL, DIEDFL_ATTACHEDONLY)))
		return hr;

	m_bJoyReady1 = true;
	if(NULL == m_pJoyDevice1)
	{
		m_bJoyReady1 = false;
		return S_OK; // 조이스틱없다고 해서 뻑낼필요없다.
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
	//  포스피드벡 모터축을 한 개 이상 발견한다면 포스피드백 이펙트를 만든다
	if(m_nForceFeedbackAxis1 >= 1)
	{
		if (m_nForceFeedbackAxis1 > 2) // 포스피드백 축은 2개만 사용
			m_nForceFeedbackAxis1 = 2;
		CreateForcefeedback();
	}

	::ZeroMemory(m_bJoy1BtnUp, sizeof(m_bJoy1BtnUp));
	::ZeroMemory(m_bJoy1BtnDn, sizeof(m_bJoy1BtnDn));
	::ZeroMemory(m_bJoy1BtnHeldDn, sizeof(m_bJoy1BtnHeldDn));

	return S_OK;
}


// Direct Input 오브젝트 및 Device 제거.
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


// 함수 설명 : 마우스 버튼 값 초기화.(Held는 제외)
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
	// 게임에 Focus가 가 있지 않다면 DirectInput을 처리하지 않는다.
	if (m_hWnd != GetForegroundWindow()) return 0;
#endif // CHECK_FOCUS

	HRESULT			hr;
	// Mouse Data를 담을 버퍼.
	DIDEVICEOBJECTDATA	didodM[MOUSE_BUFFER_SIZE];
	// Keyboard Data를 담을 버퍼.
	DIDEVICEOBJECTDATA	didodK[KEYBOARD_BUFFER_SIZE];
	DWORD			dwElements;

	// ------------------------------------------------------------------
	// Mouse 관련.
	// ------------------------------------------------------------------

	dwElements = MOUSE_BUFFER_SIZE;
	// Mouse의 Data를 얻어와 버퍼(didodM[])에 넣음.
	hr = m_pMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didodM, &dwElements, 0);

	if (hr != DI_OK)	// hr이 DI_BUFFEROVERFLOW이거나 에러코드이면.
	{
		//OutputDebugString("Mouse Input Device Losted.\n");

		// Mouse Device를 다시 획득.	
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

	// 각 축의 변화량 초기화.
	m_lDX = m_lDY = m_lDZ = 0L;
	// Down, Up, Double Click은 순간에만 TRUE가 되야되므로 FALSE로 초기화 함.
	Reset();

	// 버퍼의 Data 처리.
	for (DWORD i = 0; i < dwElements; i++)
	{
		switch (didodM[i].dwOfs)
		{
			// 각 축의 변화량 Data를 계산.
			case DIMOFS_X:
				m_lDX += long((long)didodM[i].dwData * m_fSensitivity);
				break;

			case DIMOFS_Y:
				m_lDY += long((long)didodM[i].dwData * m_fSensitivity);
				break;

			case DIMOFS_Z:
				m_lDZ += long((long)didodM[i].dwData * m_fSensitivity);
				break;

				// 각 버튼 처리.
			case DIMOFS_BUTTON0:
				if (didodM[i].dwData & 0x80)// 버튼0이 눌렸다면.
				{
					// 더블 클릭이고 이전에 원 클릭이었나.
					// 'm_bFormerBtn1Dn'는 버튼을 연타했을 때 Dbl이 교대로
					// 나오게하기 위해서.
					if (didodM[i].dwTimeStamp - m_dwBtn0LastClickTime
							<= m_dwDoubleClickTime && m_bFormerBtn0Dn)
					{
						if (m_bLeftHand) // 왼손잡이면.
						{
							m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = TRUE;
							m_bRBtnUp = FALSE;
						}
						else // 오른손잡이면.
						{
							m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = TRUE;
							m_bLBtnUp = FALSE;
						}
						m_bFormerBtn0Dn = FALSE;
					}
					else // 원 클릭이면.
					{
						if (m_bLeftHand) // 왼손잡이면.
						{
							m_bRBtnDn = m_bRBtnHeldDn = TRUE;
							m_bRBtnUp = m_bRBtnDbl = FALSE;
						}
						else // 오른손잡이면.
						{
							m_bLBtnDn = m_bLBtnHeldDn = TRUE;
							m_bLBtnUp = m_bLBtnDbl = FALSE;
						}
						m_bFormerBtn0Dn = TRUE;
					}
					m_dwBtn0LastClickTime = didodM[i].dwTimeStamp;
				}
				else // 버튼0이 올라왔다면.
				{
					if (m_bLeftHand) // 왼손잡이면.
					{
						m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = FALSE;
						m_bRBtnUp = TRUE;
					}
					else // 오른손잡이면.
					{
						m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = FALSE;
						m_bLBtnUp = TRUE;
					}
				}
				break;

			case DIMOFS_BUTTON1:
				if (didodM[i].dwData & 0x80)// 버튼1이 눌렸다면.
				{
					// 더블 클릭이고 이전에 원 클릭이었나.
					// 'm_bFormerBtn1Dn'는 버튼을 연타했을 때 Dbl이 교대로
					// 나오게하기 위해서.
					if (didodM[i].dwTimeStamp - m_dwBtn1LastClickTime
							<= m_dwDoubleClickTime && m_bFormerBtn1Dn)
					{
						if (m_bLeftHand) // 왼손잡이면.
						{
							m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = TRUE;
							m_bLBtnUp = FALSE;
						}
						else // 오른손잡이면.
						{
							m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = TRUE;
							m_bRBtnUp = FALSE;
						}
						m_bFormerBtn1Dn = FALSE;
					}
					else // 원 클릭이면.
					{
						if (m_bLeftHand) // 왼손잡이면.
						{
							m_bLBtnDn = m_bLBtnHeldDn = TRUE;
							m_bLBtnUp = m_bLBtnDbl = FALSE;
						}
						else // 오른손잡이면.
						{
							m_bRBtnDn = m_bRBtnHeldDn = TRUE;
							m_bRBtnUp = m_bRBtnDbl = FALSE;
						}
						m_bFormerBtn1Dn = TRUE;
					}
					m_dwBtn1LastClickTime = didodM[i].dwTimeStamp;
				}
				else // 버튼1이 올라왔다면.
				{
					if (m_bLeftHand) // 왼손잡이면.
					{
						m_bLBtnDn = m_bLBtnHeldDn = m_bLBtnDbl = FALSE;
						m_bLBtnUp = TRUE;
					}
					else // 오른손잡이면.
					{
						m_bRBtnDn = m_bRBtnHeldDn = m_bRBtnDbl = FALSE;
						m_bRBtnUp = TRUE;
					}
				}
				break;

			case DIMOFS_BUTTON2: // 휠 버튼.
				if (didodM[i].dwData & 0x80) // 버튼2가 눌렸다면.
				{
					// 더블 클릭이고 이전에 원 클릭이었나.
					// 'm_bFormerBtn2Dn'는 버튼을 연타했을 때 Dbl이 교대로
					// 나오게하기 위해서.
					if (didodM[i].dwTimeStamp - m_dwBtn2LastClickTime
							<= m_dwDoubleClickTime && m_bFormerBtn2Dn)
					{
						m_bMBtnDbl = TRUE;
						m_bFormerBtn2Dn = FALSE;
					}
					else // 원 클릭이면.
					{
						m_bFormerBtn2Dn = TRUE;
						m_bMBtnDbl = FALSE;
					}
					m_bMBtnDn = m_bMBtnHeldDn = TRUE;
					m_bMBtnUp = FALSE;
					m_dwBtn2LastClickTime = didodM[i].dwTimeStamp;
				}
				else // 버튼2가 올라왔다면.
				{
					m_bMBtnDn = m_bMBtnHeldDn = m_bMBtnDbl = FALSE;
					m_bMBtnUp = TRUE;
				}
				break;
		}	// switch문.
	}	// for문.

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


	// X, Y축의 변화량을 좌표에 적용.
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

	// X, Y좌표값을 제한함.
	m_lXCoord = LIMIT(m_lXCoord, 0, m_lScreenWidth - 1);
	m_lYCoord = LIMIT(m_lYCoord, 0, m_lScreenHeight - 1);

	// 기준 resolution 으로 normalize 한다
	m_lXCoord = (long)((float)m_lXCoord/(float)m_lScreenWidth*(float)NORM_RES_W);
	m_lYCoord = (long)((float)m_lYCoord/(float)m_lScreenHeight*(float)NORM_RES_H);

	// ------------------------------------------------------------------
	// Keyboard 관련.
	// ------------------------------------------------------------------

	// Up과 Dn은 유지되는 경우가 없으므로 초기화.
	// HeldDown은 누르고 있는 동안(Up이 되기 전까지) 유지되므로 초기화를 안함.
	::ZeroMemory(m_bKeyUp, sizeof(m_bKeyUp));
	::ZeroMemory(m_bKeyDn, sizeof(m_bKeyDn));
	::ZeroMemory(m_bKeyDlkDn, sizeof(m_bKeyDlkDn));

	dwElements = KEYBOARD_BUFFER_SIZE;
	// Keyboard의 Data를 얻어와 버퍼(didodK[])에 넣음.
	hr = m_pKeyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), didodK, &dwElements, 0 );

	if (hr != DI_OK) // hr이 DI_BUFFEROVERFLOW이거나 에러코드이면.
	{
		//OutputDebugString("Keyboard Input Device Losted.\n");

		// Mouse Device를 다시 획득.	
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


	// 버퍼의 Data 처리.
	for (int i = 0; i < (int)dwElements; i++)
	{
		if (didodK[i].dwData & 0x80)	// Key Down이면.
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
				// 더블클릭은 한순간에 하나만 가능하도록 한다
				::ZeroMemory(m_fKeyLastDnTime, sizeof(m_fKeyLastDnTime));
				m_fKeyLastDnTime[didodK[i].dwOfs] = (float)didodK[i].dwTimeStamp;
			}

			if (m_procKeyInputHooker) m_procKeyInputHooker(didodK[i].dwOfs, didodK[i].dwTimeStamp, true);
		}
		else	// Key Up이면.
		{
			m_bKeyHeldDn[didodK[i].dwOfs] = false;
			m_bKeyDn[didodK[i].dwOfs] = false;
			m_bKeyUp[didodK[i].dwOfs] = true;
			m_bKeyDlkDn[didodK[i].dwOfs] = false;
			m_bKeyDlkHeldDn[didodK[i].dwOfs] = false;

			if (m_procKeyInputHooker) m_procKeyInputHooker(didodK[i].dwOfs, didodK[i].dwTimeStamp, false);
		}
	}

	// 해당 Mouse Event에 따른 처리를 한다.

	// TODO: 더블클릭의 구현
	// 생각해볼것. 더블클릭은 각 오브젝트가 검사하는 것이 좋을 것 같기도 하다. 왜냐하면 여기서 검사하는 것으로는
	// 같은 오브젝트를 더블 클릭했는지 알 수 없기 때문이다.	

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

	//if ((m_lXCoord - lPrevXCoord) != 0 || (m_lYCoord - lPrevYCoord) != 0) // 이동의 변화가 있다면
	//	CMouseEventGenerator::GetInstance()->OnMouseMove((int)m_lXCoord, (int)m_lYCoord);

	//if (m_lDZ) // 휠처리
	//	CMouseEventGenerator::GetInstance()->OnMouseWheel((int)m_lXCoord, (int)m_lYCoord, m_lDZ);

	// NOTE : Mouse의 경우, DirectInput과 Window Message의 Receiver가 같지만,
	// Keyboard의 경우는 Receiver가 다르다. 이렇게 다르게 처리하는 이유는, VK 와 DIK code가 서로 다르고,
	// IME를 처리하는데 있어서 깔끔하지 않기 때문이다. 따라서 Keyboard의 경우 Receiver를 다르게 한다.

	// 해당 Key Event에 따른 처리를 한다.
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
	// Joystic 관련.
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
			if (js.rgbButtons[i] & 0x80) // 다운
			{
				if (m_bJoy1BtnHeldDn[i] == true) // 이전에도 다운
					m_bJoy1BtnDn[i] = false;
				else
					m_bJoy1BtnDn[i] = true;
				m_bJoy1BtnUp[i] = false;
				m_bJoy1BtnHeldDn[i] = true;
			}
			else // 업
			{
				if (m_bJoy1BtnHeldDn[i] == false) // 이전에도 업
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
	eff.dwDuration              = (DWORD)(300000); // 이펙트 적용 시간
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
