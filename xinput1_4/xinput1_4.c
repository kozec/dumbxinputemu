#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "dumbxinputemu.h"
#include <stdint.h>
#include <sal.h>

BOOL APIENTRY DllMain(HANDLE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        dumb_Init(DUMBINPUT_V1_4);
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
        dumb_Cleanup();
    return TRUE;
}

void WINAPI XInputEnable(
    _In_  BOOL enable
    ){
    dumb_XInputEnable(enable);
}

DWORD WINAPI XInputGetCapabilities(
    _In_   DWORD dwUserIndex,
    _In_   DWORD dwFlags,
    _Out_  XINPUT_CAPABILITIES *pCapabilities

    ){
    return dumb_XInputGetCapabilities(dwUserIndex, dwFlags, pCapabilities, DUMBINPUT_V1_4);
}

DWORD WINAPI XInputGetState(
    _In_   DWORD dwUserIndex,
    _Out_  XINPUT_STATE *pState

    ){
    return dumb_XInputGetState(dwUserIndex, pState, DUMBINPUT_V1_4);
}

DWORD WINAPI XInputSetState(
    _In_     DWORD dwUserIndex,
    _Inout_  XINPUT_VIBRATION *pVibration
    ){
    return dumb_XInputSetState(dwUserIndex, pVibration, DUMBINPUT_V1_4);
}

DWORD WINAPI XInputGetKeystroke(
    DWORD dwUserIndex,
    DWORD dwReserved,
    PXINPUT_KEYSTROKE pKeystroke){
    return dumb_XInputGetKeystroke(dwUserIndex, dwReserved, pKeystroke, DUMBINPUT_V1_4);
}

DWORD WINAPI XInputGetBatteryInformation(
    _In_   DWORD dwUserIndex,
    _In_   BYTE devType,
    _Out_  XINPUT_BATTERY_INFORMATION *pBatteryInformation
    ){
    return dumb_XInputGetBatteryInformation(dwUserIndex, devType, pBatteryInformation, DUMBINPUT_V1_4);
}

DWORD WINAPI XInputGetAudioDeviceIds(
    _In_         DWORD dwUserIndex,
    _Out_opt_    LPWSTR pRenderDeviceId,
    _Inout_opt_  UINT *pRenderCount,
    _Out_opt_    LPWSTR pCaptureDeviceId,
    _Inout_opt_  UINT *pCaptureCount
    ){

    return dumb_XInputGetAudioDeviceIds(
        dwUserIndex, pRenderDeviceId, pRenderCount, pCaptureDeviceId, pCaptureCount, DUMBINPUT_V1_4
        );
}
