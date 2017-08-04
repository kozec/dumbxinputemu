Dumb xinput Emulator
====================

xinput dll reimplementation compatibile with DirectInput controllers. Think x360ce without configuration.

##### Usage
[Download latest release](https://github.com/kozec/dumbxinputemu/releases/latest), unpack xinput1_3.dll next to game executable and start the game. For recent games, try copying xinput9_1_0.dll as well if first one doesn't works.

##### Why in the...?
This is meant mainly for using Steam Controller or XBox 360 pad with Wine.

##### Building
- Grab `mingw-w64-gcc` package or your distro equivalent containing `i686-w64-mingw32-gcc` binary
- navigate to directory with Makefile
- run `make` or `make 64bit` for 64bit version

##### Credits
Based on xinput1_3.dll implementation in Wine, [wine-xinput patch by 00cpxxx](https://github.com/00cpxxx/wine-xinput) and [xfakeinput by NeonMan](https://github.com/NeonMan/xfakeinput)
