#ifndef _dumbxinputemu_h
#define _dumbxinputemu_h

//Version ID definitions
#define DUMBINPUT_V1_1   11
#define DUMBINPUT_V1_2   12
#define DUMBINPUT_V1_3   13
#define DUMBINPUT_V1_4   14
#define DUMBINPUT_V9_1_0 910

#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include <stdbool.h>
#include <stdint.h>
#include <Xinput.h>

#ifndef XINPUT_GAMEPAD_GUIDE
    #define XINPUT_GAMEPAD_GUIDE 0x0400
#endif

typedef struct _XINPUT_GAMEPAD_EX {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
    DWORD dwPaddingReserved;
} XINPUT_GAMEPAD_EX, *PXINPUT_GAMEPAD_EX;

typedef struct _XINPUT_STATE_EX {
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD_EX Gamepad;
} XINPUT_STATE_EX, *PXINPUT_STATE_EX;

void dumb_Init(DWORD version);
void dumb_Cleanup();

void dumb_XInputEnable(BOOL enable);
DWORD dumb_XInputGetCapabilities(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES *pCapabilities, DWORD caller_version);
DWORD dumb_XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex, GUID* pDSoundRenderGuid, GUID* pDSoundCaptureGuid, DWORD caller_version);
DWORD dumb_XInputGetState(DWORD dwUserIndex, XINPUT_STATE *pState, DWORD caller_version);
DWORD dumb_XInputGetStateEx(DWORD dwUserIndex, XINPUT_STATE_EX *pState_ex, DWORD caller_version);
DWORD dumb_XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration, DWORD caller_version);
DWORD dumb_XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke, DWORD caller_version);
DWORD dumb_XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION *pBatteryInformation, DWORD caller_version);
DWORD dumb_XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId, UINT *pRenderCount, LPWSTR pCaptureDeviceId, UINT *pCaptureCount, DWORD caller_version);

#endif // _dumbxinputemu_h