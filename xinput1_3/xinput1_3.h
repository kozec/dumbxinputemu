/*
 * The Wine project - Xinput Joystick Library
 * Copyright 2008 Andrew Fenn
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

#ifndef __XINPUT1_3_H
#define __XINPUT1_3_H

#include <windef.h>

#define EXPORT __declspec(dllexport)

/*
 * Bitmasks for the joysticks buttons, determines what has
 * been pressed on the joystick, these need to be mapped
 * to whatever device you're using instead of an xbox 360
 * joystick
 */

#define XINPUT_GAMEPAD_DPAD_UP          0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN        0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT        0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT       0x0008
#define XINPUT_GAMEPAD_START            0x0010
#define XINPUT_GAMEPAD_BACK             0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB       0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB      0x0080
#define XINPUT_GAMEPAD_LEFT_SHOULDER    0x0100
#define XINPUT_GAMEPAD_RIGHT_SHOULDER   0x0200
#define XINPUT_GAMEPAD_A                0x1000
#define XINPUT_GAMEPAD_B                0x2000
#define XINPUT_GAMEPAD_X                0x4000
#define XINPUT_GAMEPAD_Y                0x8000

/*
 * Defines the flags used to determine if the user is pushing
 * down on a button, not holding a button, etc
 */

#define XINPUT_KEYSTROKE_KEYDOWN        0x0001
#define XINPUT_KEYSTROKE_KEYUP          0x0002
#define XINPUT_KEYSTROKE_REPEAT         0x0004

/*
 * Defines what type of abilities the type of joystick has
 * DEVTYPE_GAMEPAD is available for all joysticks, however
 * there may be more specific identifiers for other joysticks
 * which are being used.
 */

#define XINPUT_DEVTYPE_GAMEPAD          0x01
#define XINPUT_DEVSUBTYPE_GAMEPAD       0x01
#define XINPUT_DEVSUBTYPE_WHEEL         0x02
#define XINPUT_DEVSUBTYPE_ARCADE_STICK  0x03
#define XINPUT_DEVSUBTYPE_FLIGHT_SICK   0x04
#define XINPUT_DEVSUBTYPE_DANCE_PAD     0x05
#define XINPUT_DEVSUBTYPE_GUITAR        0x06
#define XINPUT_DEVSUBTYPE_DRUM_KIT      0x08

/*
 * These are used with the XInputGetCapabilities function to
 * determine the abilities to the joystick which has been
 * plugged in.
 */

#define XINPUT_CAPS_VOICE_SUPPORTED     0x0004
#define XINPUT_FLAG_GAMEPAD             0x00000001

/*
 * Defines the status of the battery if one is used in the
 * attached joystick. The first two define if the joystick
 * supports a battery. Disconnected means that the joystick
 * isn't connected. Wired shows that the joystick is a wired
 * joystick.
 */

#define BATTERY_DEVTYPE_GAMEPAD         0x00
#define BATTERY_DEVTYPE_HEADSET         0x01
#define BATTERY_TYPE_DISCONNECTED       0x00
#define BATTERY_TYPE_WIRED              0x01
#define BATTERY_TYPE_ALKALINE           0x02
#define BATTERY_TYPE_NIMH               0x03
#define BATTERY_TYPE_UNKNOWN            0xFF
#define BATTERY_LEVEL_EMPTY             0x00
#define BATTERY_LEVEL_LOW               0x01
#define BATTERY_LEVEL_MEDIUM            0x02
#define BATTERY_LEVEL_FULL              0x03

/*
 * How many joysticks can be used with this library. Games that
 * use the xinput library will not go over this number.
 */

#define XUSER_MAX_COUNT                 4
#define XUSER_INDEX_ANY                 0x000000FF

/*
 * Defines the structure of an xbox 360 joystick.
 */

typedef struct _XINPUT_GAMEPAD {
    WORD wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

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

typedef struct _XINPUT_STATE {
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef struct _XINPUT_STATE_EX {
    DWORD dwPacketNumber;
    XINPUT_GAMEPAD_EX Gamepad;
} XINPUT_STATE_EX, *PXINPUT_STATE_EX;

/*
 * Defines the structure of how much vibration is set on both the
 * right and left motors in a joystick. If you're not using a 360
 * joystick you will have to map these to your device.
 */

typedef struct _XINPUT_VIBRATION {
    WORD wLeftMotorSpeed;
    WORD wRightMotorSpeed;
} XINPUT_VIBRATION, *PXINPUT_VIBRATION;

/*
 * Defines the structure for what kind of abilities the joystick has
 * such abilities are things such as if the joystick has the ability
 * to send and receive audio, if the joystick is in fact a driving
 * wheel or perhaps if the joystick is some kind of dance pad or
 * guitar.
 */

typedef struct _XINPUT_CAPABILITIES {
    BYTE Type;
    BYTE SubType;
    WORD Flags;
    XINPUT_GAMEPAD Gamepad;
    XINPUT_VIBRATION Vibration;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

/*
 * Defines the structure for a joystick input event which is
 * retrieved using the function XInputGetKeystroke
 */
typedef struct _XINPUT_KEYSTROKE {
    WORD VirtualKey;
    WCHAR Unicode;
    WORD Flags;
    BYTE UserIndex;
    BYTE HidCode;
} XINPUT_KEYSTROKE, *PXINPUT_KEYSTROKE;

typedef struct _XINPUT_BATTERY_INFORMATION
{
    BYTE BatteryType;
    BYTE BatteryLevel;
} XINPUT_BATTERY_INFORMATION, *PXINPUT_BATTERY_INFORMATION;

#ifdef __cplusplus
extern "C" {
#endif

void EXPORT XInputEnable(BOOL);
DWORD EXPORT XInputSetState(DWORD, XINPUT_VIBRATION*);
DWORD EXPORT XInputGetState(DWORD, XINPUT_STATE*);
DWORD EXPORT XInputGetKeystroke(DWORD, DWORD, PXINPUT_KEYSTROKE);
DWORD EXPORT XInputGetCapabilities(DWORD, DWORD, XINPUT_CAPABILITIES*);
DWORD EXPORT XInputGetDSoundAudioDeviceGuids(DWORD, GUID*, GUID*);
DWORD EXPORT XInputGetBatteryInformation(DWORD, BYTE, XINPUT_BATTERY_INFORMATION*);

DWORD EXPORT XInputGetStateEx(DWORD, XINPUT_STATE_EX*);

#ifdef __cplusplus
}
#endif

#endif /* __XINPUT1_3_H */
