# mingw32-gcc
GCC = /usr/bin/i686-w64-mingw32-gcc -D _WXI_MINGW
INCLUDE_DIR = -I/usr/i686-w64-mingw32/include 
CFLAGS = -Wl,--export-all-symbols
LIB_DIR = -L/usr/i686-w64-mingw32/lib/
LIBS = -ldxguid -ldinput -ldinput8

xinput1_3.dll: xinput1_3/xinput1_3_main.c
	${GCC} -shared ${CFLAGS} ${INCLUDE_DIR} ${LIB_DIR} $? ${LIBS} -o $@

clean:
	rm -f *.o *.dll

all: xinput1_3.dll
