Dumb xinput Emulator
====================

This is an xinput dll reimplementation compatible with DirectInput controllers. Think of x360ce without the configuration.

##### Usage
- download and extract the [latest release](https://github.com/kozec/dumbxinputemu/releases/latest)
- if you are using Wine run `winetricks --force setup_dumbxinputemu.verb`
- otherwise copy all xinputXYZ.dll's next to the game executable and start the game
- on Wine, dumbxinputemu uses evdev and ignores jsdev devices by default. That should work with almost everything,
  but you can control this behavior using `XINPUT_NO_IGNORE_JS` and `XINPUT_IGNORE_EVDEV` environment variables.

##### Why in the...?
This is meant mainly for using a Steam Controller or XBox 360 pad with Wine.

##### Building
- grab `mingw-w64-gcc` package or your distro equivalent containing an `i686-w64-mingw32-gcc` binary
- navigate to the directory with `Makefile`
- run `make`, or `make 64bit` for the 64-bit version

##### Credits
Based on xinput1_3.dll implementation in Wine, [wine-xinput patch by 00cpxxx](https://github.com/00cpxxx/wine-xinput) and [xfakeinput by NeonMan](https://github.com/NeonMan/xfakeinput).
