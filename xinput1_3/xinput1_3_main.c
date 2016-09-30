/*
 * The Wine project - Xinput Joystick Library
 *
 * Copyright 2008 Andrew Fenn
 * Copyright 2016 Bruno Jesus
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define COBJMACROS
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#ifdef __WINEGCC__
	// Available only with winegcc
	#include "wine/debug.h"
	WINE_DEFAULT_DEBUG_CHANNEL(xinput);
#endif
#include "windef.h"
#include "winbase.h"
#include "winerror.h"
#include "unknwn.h"

#include "dinput.h"
#ifdef __MINGW32__
	// Mingw compiles weird mutation with no usable symbols without this
	// TODO: I don't really know why this helps
	#undef WINAPI
	#define WINAPI
#endif
#include "xinput.h"

#ifndef TRACE
	// Unavailable outside wine
	#define TRACE(...) do { } while(0)
	#define FIXME(...) do { } while(0)
	#define WARN(...)  do { } while(0)
	#define ERR(...)   do { } while(0)
#endif

#ifdef __MINGW32__
	// Stuff missing in mingw
	typedef struct {
		WORD wButtons;
		BYTE bLeftTrigger;
		BYTE bRightTrigger;
		SHORT sThumbLX;
		SHORT sThumbLY;
		SHORT sThumbRX;
		SHORT sThumbRY;
		DWORD dwPaddingReserved;
	} XINPUT_GAMEPAD_EX;

	typedef struct {
		DWORD dwPacketNumber;
		XINPUT_GAMEPAD_EX Gamepad;
	} XINPUT_STATE_EX;
#endif

/* Not defined in the headers, used only by XInputGetStateEx */
#define XINPUT_GAMEPAD_GUIDE 0x0400


struct CapsFlags
{
    BOOL wireless, jedi, pov;
    int axes, buttons;
};

static struct ControllerMap
{
    LPDIRECTINPUTDEVICE8A device;
    BOOL connected, acquired;
    struct CapsFlags caps;
    XINPUT_STATE state;
    XINPUT_VIBRATION vibration;
    BOOL vibration_dirty;

    DIEFFECT effect_data;
    LPDIRECTINPUTEFFECT effect_instance;
} controllers[XUSER_MAX_COUNT];

static struct
{
    LPDIRECTINPUT8A iface;
    BOOL enabled;
    int mapped;
} dinput;

#define STARTUP_DINPUT if (!dinput.iface) dinput_start();

/* ========================= Internal functions ============================= */

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

static void dinput_joystate_to_xinput(DIJOYSTATE2 *js, XINPUT_GAMEPAD *gamepad, struct CapsFlags *caps)
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
        0, /* xbox key not used */
        XINPUT_GAMEPAD_LEFT_THUMB,
        XINPUT_GAMEPAD_RIGHT_THUMB
    };
    int i, buttons;

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
        gamepad->sThumbRX = gamepad->sThumbRY = 0;

    /* Both triggers */
    if (caps->axes >= 6)
    {
        gamepad->bLeftTrigger = (255 * (js->lZ + 32767)) / 32767;
        gamepad->bRightTrigger = (255 * (js->lRz + 32767)) / 32767;
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
            IUnknown_Release(*instance);
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
    XINPUT_GAMEPAD gamepad;

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
        memset(&gamepad, 0, sizeof(gamepad));

    if (memcmp(&controllers[index].state.Gamepad, &gamepad, sizeof(gamepad)))
    {
        controllers[index].state.Gamepad = gamepad;
        controllers[index].state.dwPacketNumber++;
    }
}

/* ============================ Dll Functions =============================== */


BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID reserved)
{
    switch(reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(inst);
        break;
    }
    return TRUE;
}

void WINAPI XInputEnable(BOOL enable)
{
    /* Setting to false will stop messages from XInputSetState being sent
    to the controllers. Setting to true will send the last vibration
    value (sent to XInputSetState) to the controller and allow messages to
    be sent */
    TRACE("(%d)\n", enable);

    STARTUP_DINPUT

    if((dinput.enabled = enable))
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

DWORD WINAPI XInputSetState(DWORD index, XINPUT_VIBRATION* vibration)
{
    TRACE("(%u %p)\n", index, vibration);

    STARTUP_DINPUT

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

DWORD WINAPI XInputGetState(DWORD index, XINPUT_STATE* state)
{
    union
    {
        XINPUT_STATE state;
        XINPUT_STATE_EX state_ex;
    } xinput;
    DWORD ret;

    TRACE("(%u %p)\n", index, state);
    STARTUP_DINPUT

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
         return ERROR_DEVICE_NOT_CONNECTED;


    dinput_update(index);
    *state = controllers[index].state;

    /* The main difference between this and the Ex version is the media guide button */
    /* TODO: Check on this.
     * xinput.state.Gamepad.wButtons &= ~XINPUT_GAMEPAD_GUIDE;
     * *state = xinput.state;
     */

    return ERROR_SUCCESS;
}

DWORD WINAPI XInputGetStateEx(DWORD index, XINPUT_STATE_EX* state_ex)
{
    static int warn_once;

    if (!warn_once++)
        FIXME("(index %u, state %p) Stub!\n", index, state_ex);

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    return ERROR_NOT_SUPPORTED;
}

DWORD WINAPI XInputGetKeystroke(DWORD index, DWORD reserved, PXINPUT_KEYSTROKE keystroke)
{
    FIXME("(index %u, reserved %u, keystroke %p) Stub!\n", index, reserved, keystroke);

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    return ERROR_NOT_SUPPORTED;
}

/* Not defined anywhere ??? */
#define XINPUT_CAPS_FFB_SUPPORTED 0x0001
#define XINPUT_CAPS_WIRELESS      0x0002
#define XINPUT_CAPS_NO_NAVIGATION 0x0010

DWORD WINAPI XInputGetCapabilities(DWORD index, DWORD flags, XINPUT_CAPABILITIES* capabilities)
{
    TRACE("(%u %d %p)\n", index, flags, capabilities);

    STARTUP_DINPUT

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    capabilities->Type = XINPUT_DEVTYPE_GAMEPAD;
    capabilities->SubType = XINPUT_DEVSUBTYPE_GAMEPAD;

    capabilities->Flags = 0;
    if (controllers[index].caps.jedi)
        capabilities->Flags |= XINPUT_CAPS_FFB_SUPPORTED;
    if (controllers[index].caps.wireless)
        capabilities->Flags |= XINPUT_CAPS_WIRELESS;
    if (!controllers[index].caps.pov)
        capabilities->Flags |= XINPUT_CAPS_NO_NAVIGATION;

    dinput_update(index);

    capabilities->Vibration = controllers[index].vibration;
    capabilities->Gamepad = controllers[index].state.Gamepad;

    return ERROR_SUCCESS;
}

DWORD WINAPI XInputGetDSoundAudioDeviceGuids(DWORD index, GUID* render_guid, GUID* capture_guid)
{
    FIXME("(index %u, render guid %p, capture guid %p) Stub!\n", index, render_guid, capture_guid);

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    return ERROR_NOT_SUPPORTED;
}

DWORD WINAPI XInputGetBatteryInformation(DWORD index, BYTE type, XINPUT_BATTERY_INFORMATION* battery)
{
    TRACE("(%u %u %p) Stub!\n", index, type, battery);

    STARTUP_DINPUT

    if (index >= XUSER_MAX_COUNT)
        return ERROR_BAD_ARGUMENTS;
    if (!controllers[index].connected)
        return ERROR_DEVICE_NOT_CONNECTED;

    battery->BatteryType = BATTERY_TYPE_WIRED;
    battery->BatteryLevel = BATTERY_LEVEL_FULL;


    return ERROR_SUCCESS;
}
