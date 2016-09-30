# mingw32-gcc
CFLAGS = -Wl,--export-all-symbols
LIBS = -ldxguid -ldinput -ldinput8

GCC32 = /usr/bin/i686-w64-mingw32-gcc -D _WXI_MINGW
INCLUDE_DIR32 = -I/usr/i686-w64-mingw32/include 
LIB_DIR32 = -L/usr/i686-w64-mingw32/lib/

build/32/xinput1_3.dll: xinput1_3/xinput1_3_main.c
	mkdir -p build/32/
	${GCC32} -shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

GCC64 = /usr/bin/x86_64-w64-mingw32-gcc -D _WXI_MINGW
INCLUDE_DIR64 = -I/usr/x86_64-w64-mingw32/include 
LIB_DIR64 = -L/usr/x86_64-w64-mingw32/lib/

build/64/xinput1_3.dll: xinput1_3/xinput1_3_main.c
	mkdir -p build/64/
	${GCC64} -shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

clean:
	rm -f *.o *.dll

32bit: build/32/xinput1_3.dll

64bit: build/64/xinput1_3.dll

all: build/32/xinput1_3.dll
