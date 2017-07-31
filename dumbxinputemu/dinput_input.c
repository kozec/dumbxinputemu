#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "dumbxinputemu.h"
#include <cguid.h>
#include <stdio.h>

#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

#ifndef TRACE
    // Available only in Wine
    // #define TRACE(format, ...) printf("TRACE[%d] " format, __LINE__, ## __VA_ARGS__)
    #define TRACE(...) do { } while(0)
    #define DPRINT(...) do { } while(0)
    #define FIXME(...) do { } while(0)
    #define WARN(...)  do { } while(0)
    #define ERR(format, ...) printf("ERR[%d] " format, __LINE__, ## __VA_ARGS__)
#endif

struct CapsFlags {
    BOOL wireless, jedi, pov;
    int axes, buttons;
};

static struct ControllerMap {
    LPDIRECTINPUTDEVICE8A device;
    BOOL connected, acquired;
    struct CapsFlags caps;
    XINPUT_STATE_EX state_ex;
    XINPUT_VIBRATION vibration;
    BOOL vibration_dirty;

    DIEFFECT effect_data;
    LPDIRECTINPUTEFFECT effect_instance;
} controllers[XUSER_MAX_COUNT];

static struct {
    LPDIRECTINPUT8A iface;
    BOOL enabled;
    int mapped;
} dinput;


/* ========================= Internal functions ============================= */

bool initialized = FALSE;


static BOOL dinput_is_good(const LPDIRECTINPUTDEVICE8A device, struct CapsFlags *caps)
{
    HRESULT hr;
    DIPROPDWORD property;
    DIDEVCAPS dinput_caps;
    static const unsigned long wireless_products[] = {
        MAKELONG(0x045e, 0x0291) /* microsoft receiver */,
        MAKELONG(0x045e, 0x0719) /* microsoft controller */,
        MAKELONG(0x0738, 0x4556) /* mad catz */,
        MAKELONG(0x0e6f, 0x0003) /* logitech */,
        MAKELONG(0x0e6f, 0x0005) /* eclipse */,
        MAKELONG(0x0e6f, 0x0006) /* edge */,
        MAKELONG(0x102c, 0xff0c) /* joytech */
    };
    int i;

    dinput_caps.dwSize = sizeof(dinput_caps);
    hr = IDirectInputDevice_GetCapabilities(device, &dinput_caps);
    if (FAILED(hr))
        return FALSE;

    property.diph.dwSize = sizeof(property);
    property.diph.dwHeaderSize = sizeof(property.diph);
    property.diph.dwObj = 0;
    property.diph.dwHow = DIPH_DEVICE;

    hr = IDirectInputDevice_GetProperty(device, DIPROP_VIDPID, &property.diph);
    if (FAILED(hr))
        return FALSE;

    if (dinput_caps.dwAxes < 2 || dinput_caps.dwButtons < 8)
        return FALSE;

    caps->axes = dinput_caps.dwAxes;
    caps->buttons = dinput_caps.dwButtons;
    caps->wireless = FALSE;
    caps->jedi = !!(dinput_caps.dwFlags & DIDC_FORCEFEEDBACK);
    caps->pov = !!dinput_caps.dwPOVs;

    for (i = 0; i < sizeof(wireless_products) / sizeof(wireless_products[0]); i++)
        if (property.dwData == wireless_products[i])
        {
            caps->wireless = TRUE;
            break;
        }

    if (dinput_caps.dwAxes == 6 && dinput_caps.dwButtons == 11  && dinput_caps.dwPOVs == 1)
        TRACE("This controller has the same number of buttons/axes from xbox 360, should work...\n");
    else
        FIXME("This is not a known xbox controller, using anyway. Expect problems!\n");

    return TRUE;
}


static BOOL dinput_set_range(const LPDIRECTINPUTDEVICE8A device)
{
    HRESULT hr;
    DIPROPRANGE property;

    property.diph.dwSize = sizeof(property);
    property.diph.dwHeaderSize = sizeof(property.diph);
    property.diph.dwHow = DIPH_DEVICE;
    property.diph.dwObj = 0;
    property.lMin = -32767;
    property.lMax = +32767;

    hr = IDirectInputDevice_SetProperty(device, DIPROP_RANGE, &property.diph);
    if (FAILED(hr))
    {
        WARN("Failed to set axis range (0x%x)\n", hr);
        return FALSE;
    }
        return TRUE;
}


static void dinput_joystate_to_xinput(DIJOYSTATE2 *js, XINPUT_GAMEPAD_EX *gamepad, struct CapsFlags *caps)
{
    static const int xbox_buttons[] = {
        XINPUT_GAMEPAD_A,
        XINPUT_GAMEPAD_B,
        XINPUT_GAMEPAD_X,
        XINPUT_GAMEPAD_Y,
        XINPUT_GAMEPAD_LEFT_SHOULDER,
        XINPUT_GAMEPAD_RIGHT_SHOULDER,
        XINPUT_GAMEPAD_BACK,
        XINPUT_GAMEPAD_START,
        XINPUT_GAMEPAD_GUIDE,
        XINPUT_GAMEPAD_LEFT_THUMB,
        XINPUT_GAMEPAD_RIGHT_THUMB
    };
    int i, buttons;

    gamepad->dwPaddingReserved = 0;
    gamepad->wButtons = 0x0000;
    /* First the D-Pad which is recognized as a POV in dinput */
    if (caps->pov)
    {
        switch (js->rgdwPOV[0])
        {
            case 0    : gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_UP; break;
            case 4500 : gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_UP; /* fall through */
            case 9000 : gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; break;
            case 13500: gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT; /* fall through */
            case 18000: gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN; break;
            case 22500: gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN; /* fall through */
            case 27000: gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_LEFT; break;
            case 31500: gamepad->wButtons |= XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_UP;
        }
    }

    /* Buttons */
    buttons = min(caps->buttons, sizeof(xbox_buttons) / sizeof(*xbox_buttons));
    for (i = 0; i < buttons; i++)
        if (js->rgbButtons[i] & 0x80)
            gamepad->wButtons |= xbox_buttons[i];

    /* Axes */
    gamepad->sThumbLX = js->lX;
    gamepad->sThumbLY = -js->lY;
    if (caps->axes >= 4)
    {
        gamepad->sThumbRX = js->lRx;
        gamepad->sThumbRY = -js->lRy;
    }
    else
    {
        gamepad->sThumbRX = gamepad->sThumbRY = 0;
    }

    /* Both triggers */
    if (caps->axes >= 6)
    {
        gamepad->bLeftTrigger = (255 * (long)(js->lZ + 32767)) / 65535;
        gamepad->bRightTrigger = (255 * (long)(js->lRz + 32767)) / 65535;
    }
    else
        gamepad->bLeftTrigger = gamepad->bRightTrigger = 0;
}


static void dinput_fill_effect(DIEFFECT *effect)
{
    static DWORD axes[2] = {DIJOFS_X, DIJOFS_Y};
    static LONG direction[2] = {0, 0};

    effect->dwSize = sizeof(effect);
    effect->dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    effect->dwDuration = INFINITE;
    effect->dwGain = 0;
    effect->dwTriggerButton = DIEB_NOTRIGGER;
    effect->cAxes = sizeof(axes) / sizeof(axes[0]);
    effect->rgdwAxes = axes;
    effect->rglDirection = direction;
}


static void dinput_send_effect(int index, int power)
{
    HRESULT hr;
    DIPERIODIC periodic;
    DIEFFECT *effect = &controllers[index].effect_data;
    LPDIRECTINPUTEFFECT *instance = &controllers[index].effect_instance;

    if (!*instance)
        dinput_fill_effect(effect);

    effect->cbTypeSpecificParams  = sizeof(periodic);
    effect->lpvTypeSpecificParams = &periodic;

    periodic.dwMagnitude = power;
    periodic.dwPeriod = DI_SECONDS; /* 1 second */
    periodic.lOffset = 0;
    periodic.dwPhase = 0;

    if (!*instance)
    {
        hr = IDirectInputDevice8_CreateEffect(controllers[index].device, &GUID_Square,
                                              effect, instance, NULL);
        if (FAILED(hr))
        {
            WARN("Failed to create effect (0x%x)\n", hr);
            return;
        }
        if (!*instance)
        {
            WARN("Effect not returned???\n");
            return;
        }

        hr = IDirectInputEffect_SetParameters(*instance, effect, DIEP_AXES | DIEP_DIRECTION | DIEP_NODOWNLOAD);
        if (FAILED(hr))
        {
            // TODO: I may just introduce memory leak
            // IUnknown_Release(*instance);
            *instance = NULL;
            WARN("Failed to configure effect (0x%x)\n", hr);
            return;
        }
    }

    hr = IDirectInputEffect_SetParameters(*instance, effect, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    if (FAILED(hr))
    {
        WARN("Failed to play effect (0x%x)\n", hr);
        return;
    }
}


static BOOL CALLBACK dinput_enum_callback(const DIDEVICEINSTANCEA *instance, void *context)
{
    LPDIRECTINPUTDEVICE8A device;
    HRESULT hr;

    if(strstr(instance->tszProductName, "(js)") != NULL)
    {
        // Skip 'js' devices, use only evdev
        return DIENUM_CONTINUE;
    }

    if (dinput.mapped == sizeof(controllers) / sizeof(*controllers))
        return DIENUM_STOP;

    hr = IDirectInput_CreateDevice(dinput.iface, &instance->guidInstance, &device, NULL);
    if (FAILED(hr))
        return DIENUM_CONTINUE;

    if (!dinput_is_good(device, &controllers[dinput.mapped].caps))
    {
        IDirectInput_Release(device);
        return DIENUM_CONTINUE;
    }

    if (!dinput_set_range(device))
    {
        IDirectInput_Release(device);
        return DIENUM_CONTINUE;
    }

    controllers[dinput.mapped].connected = TRUE;
    controllers[dinput.mapped].device = device;
    dinput.mapped++;

    return DIENUM_CONTINUE;
}


static void dinput_start(void)
{
    HRESULT hr;

    hr = DirectInput8Create(GetModuleHandleA(NULL), 0x0800, &IID_IDirectInput8A,
                            (void **)&dinput.iface, NULL);
    if (FAILED(hr))
    {
        ERR("Failed to create dinput8 interface, no xinput controller support (0x%x)\n", hr);
        return;
    }

    hr = IDirectInput8_EnumDevices(dinput.iface, DI8DEVCLASS_GAMECTRL,
                                   dinput_enum_callback, NULL, DIEDFL_ATTACHEDONLY);
    if (FAILED(hr))
    {
        ERR("Failed to enumerate dinput8 devices, no xinput controller support (0x%x)\n", hr);
        return;
    }

    dinput.enabled = TRUE;
}


static void dinput_update(int index)
{
    HRESULT hr;
    DIJOYSTATE2 data;
    XINPUT_GAMEPAD_EX gamepad;
    
    DPRINT("dinput_update: %d\n", index);
    
    if (dinput.enabled)
    {
        if (!controllers[index].acquired)
        {
            IDirectInputDevice8_SetDataFormat(controllers[index].device, &c_dfDIJoystick2);
            hr = IDirectInputDevice8_Acquire(controllers[index].device);
            if (FAILED(hr))
            {
                WARN("Failed to acquire game controller (0x%x)\n", hr);
                return;
            }
            controllers[index].acquired = TRUE;
        }

        IDirectInputDevice8_Poll(controllers[index].device);
        hr = IDirectInputDevice_GetDeviceState(controllers[index].device, sizeof(data), &data);
        if (FAILED(hr))
        {
            if (hr == DIERR_INPUTLOST)
                controllers[index].acquired = FALSE;
            WARN("Failed to get game controller state (0x%x)\n", hr);
            return;
        }
        dinput_joystate_to_xinput(&data, &gamepad, &controllers[index].caps);
    }
    else
        memset(&gamepad, 0, sizeof(XINPUT_GAMEPAD_EX));

    if (memcmp(&controllers[index].state_ex.Gamepad, &gamepad, sizeof(XINPUT_GAMEPAD_EX)))
    {
        controllers[index].state_ex.Gamepad = gamepad;
        controllers[index].state_ex.dwPacketNumber++;
    }
}


void dumb_Init(DWORD version)
{
    if (initialized)
        return;
    dinput_start();
    initialized = TRUE;
}

void dumb_Cleanup()
{
    // Does nothing
}

/* ============================ Dll Functions =============================== */

DWORD dumb_XInputGetState(DWORD index, XINPUT_STATE *state, DWORD caller_version)
{
    union
    {
        XINPUT_STATE state;
        XINPUT_STATE_EX state_ex;
    } xinput;
    DWORD ret;
    
    DPRINT("XInputGetState: %d\n", index);

    ret = dumb_XInputGetStateEx(index, &xinput.state_ex, caller_version);
    if (ret != ERROR_SUCCESS)
        return ret;

    /* The main difference between this and the Ex version is the media guide button */
    *state = xinput.state;
    state->Gamepad.wButtons &= ~XINPUT_GAMEPAD_GUIDE;

    return ERROR_SUCCESS;
}


DWORD dumb_XInputGetStateEx(DWORD index, XINPUT_STATE_EX *state_ex, DWORD caller_version)
{
    TRACE("(%u %p)\n", index, state_ex);
    DPRINT("XInputGetStateEx: %d\n", index);

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    dinput_update(index);
    // broforce does not pass a correct XINPUT_STATE_EX, so only copy the old struct size
    *state_ex = controllers[index].state_ex;

    return ERROR_SUCCESS;
}


DWORD dumb_XInputSetState(DWORD index, XINPUT_VIBRATION *vibration, DWORD caller_version)
{
    // TRACE("(%u %p)\n", index, vibration);
    
    DPRINT("XInputSetState: %d\n", index);

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    /* Check if we really have to do all the process */
    if (!controllers[index].vibration_dirty &&
        !memcmp(&controllers[index].vibration, vibration, sizeof(*vibration)))
        return ERROR_SUCCESS;

    controllers[index].vibration = *vibration;
    controllers[index].vibration_dirty = !dinput.enabled;

    if (dinput.enabled && controllers[index].caps.jedi)
    {
        int power;
        /* FIXME: we can't set the speed of each motor so do an average */
        power = DI_FFNOMINALMAX * (vibration->wLeftMotorSpeed + vibration->wRightMotorSpeed) / 2 / 0xFFFF;

        TRACE("Vibration left/right speed %d/%d translated to %d\n\n",
              vibration->wLeftMotorSpeed, vibration->wRightMotorSpeed, power);
        dinput_send_effect(index, power);
    }

    return ERROR_SUCCESS;
}


void dumb_XInputEnable(BOOL enable)
{
    /* Setting to false will stop messages from XInputSetState being sent
    to the controllers. Setting to true will send the last vibration
    value (sent to XInputSetState) to the controller and allow messages to
    be sent */
    TRACE("(%d)\n", enable);
    
    DPRINT("XInputEnable: %d\n", enable);

    if ((dinput.enabled = enable))
    {
        int i;
        /* Apply the last vibration status that was sent to the controller
         * while xinput was disabled. */
        for (i = 0; i < sizeof(controllers) / sizeof(*controllers); i++)
        {
            if (controllers[i].connected && controllers[i].vibration_dirty)
                XInputSetState(i, &controllers[i].vibration);
        }
    }
}

/* Not defined anywhere ??? */
#define XINPUT_CAPS_FFB_SUPPORTED 0x0001
#define XINPUT_CAPS_WIRELESS      0x0002
#define XINPUT_CAPS_NO_NAVIGATION 0x0010

DWORD dumb_XInputGetCapabilities(DWORD index, DWORD flags,
        XINPUT_CAPABILITIES *capabilities, DWORD caller_version)
{
    TRACE("(%u %d %p)\n", index, flags, capabilities);
    
    DPRINT("XInputGetCapabilities: %d\n", index);

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    capabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
    capabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;

    capabilities->Flags = 0;
    if (controllers[index].caps.jedi)
        capabilities->Flags |= XINPUT_CAPS_FFB_SUPPORTED;
    if (!controllers[index].caps.pov)
        capabilities->Flags |= XINPUT_CAPS_NO_NAVIGATION;

    dinput_update(index);

    capabilities->Vibration = controllers[index].vibration;
    capabilities->Gamepad = *(XINPUT_GAMEPAD *)&controllers[index].state_ex.Gamepad;

    return ERROR_SUCCESS;
}


DWORD dumb_XInputGetDSoundAudioDeviceGuids(DWORD dwUserIndex, GUID* pDSoundRenderGuid,
        GUID* pDSoundCaptureGuid, DWORD caller_version)
{
    if (dwUserIndex > 3)
        return ERROR_DEVICE_NOT_CONNECTED;
    *pDSoundRenderGuid = GUID_NULL;
    *pDSoundCaptureGuid = GUID_NULL;
    return ERROR_SUCCESS;
}


DWORD dumb_XInputGetKeystroke(DWORD dwUserIndex, DWORD dwReserved,
        PXINPUT_KEYSTROKE pKeystroke, DWORD caller_version)
{
    FIXME("(index %u, reserved %u, keystroke %p) Stub!\n", dwUserIndex,
            dwReserved, pKeystroke);
    if (dwUserIndex > 3)
        return ERROR_DEVICE_NOT_CONNECTED;
    return ERROR_EMPTY;
}


DWORD dumb_XInputGetBatteryInformation(DWORD dwUserIndex, BYTE devType,
        XINPUT_BATTERY_INFORMATION *pBatteryInformation, DWORD caller_version)
{
    pBatteryInformation->BatteryLevel = BATTERY_LEVEL_FULL;
    pBatteryInformation->BatteryType = BATTERY_TYPE_WIRED;
    return ERROR_SUCCESS;
}


DWORD dumb_XInputGetAudioDeviceIds(DWORD dwUserIndex, LPWSTR pRenderDeviceId,
        UINT *pRenderCount, LPWSTR pCaptureDeviceId,
        UINT *pCaptureCount, DWORD caller_version)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}
