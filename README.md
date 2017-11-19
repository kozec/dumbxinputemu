Dumb xinput Emulator
====================

This is an xinput dll reimplementation compatible with DirectInput controllers. Think of x360ce without the configuration.

##### Usage
- [download the latest release](https://github.com/kozec/dumbxinputemu/releases/latest)
- unpack `xinput1_3.dll` next to the game executable and start the game
  - (for recent games, try copying `xinput9_1_0.dll` as well if the above doesn't work)
- if you are using WINE, don't forget to add DLL override(s) and set them to 'native'

##### Why in the...?
This is meant mainly for using a Steam Controller or XBox 360 pad with Wine.

##### Building
- grab `mingw-w64-gcc` package or your distro equivalent containing an `i686-w64-mingw32-gcc` binary
- navigate to the directory with `Makefile`
- run `make`, or `make 64bit` for the 64-bit version

##### Credits
Based on xinput1_3.dll implementation in Wine, [wine-xinput patch by 00cpxxx](https://github.com/00cpxxx/wine-xinput) and [xfakeinput by NeonMan](https://github.com/NeonMan/xfakeinput).
