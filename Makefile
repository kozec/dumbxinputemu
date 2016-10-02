# Important stuff
CFLAGS = 
LIBS = -ldxguid -ldinput -ldinput8

# (maybe) distro-specific stuff
MINGW32 = i686-w64-mingw32
MINGW64 = i686-w64-mingw32
BIN = /usr/bin
USR = /usr
LIB = lib/			# used in as ${USR}/${MINGWxy}/${LIB}
INCLUDE = include/	# used in as ${USR}/${MINGWxy}/${INCLUDE}

# Computed
GCC32 = ${BIN}/${MINGW32}-gcc
INCLUDE_DIR32 = -I${USR}/${MINGW32}/include 
LIB_DIR32 = -L{USR}/${MINGW32}/lib/

GCC64 = ${BIN}/x86_64-w64-mingw32-gcc -D _WXI_MINGW
INCLUDE_DIR64 = -I${USR}/${MINGW64}/include 
LIB_DIR64 = -L{USR}/${MINGW64}/lib/

# ---- Targets -------

build/32/xinput1_3.dll: xinput1_3/xinput1_3.def xinput1_3/xinput1_3_main.c
	mkdir -p build/32/
	${GCC32} --shared ${CFLAGS} ${INCLUDE_DIR32} ${LIB_DIR32} $? ${LIBS} -o $@

build/64/xinput1_3.dll: xinput1_3/xinput1_3.def xinput1_3/xinput1_3_main.c
	mkdir -p build/64/
	${GCC64} -shared ${CFLAGS} ${INCLUDE_DIR64} ${LIB_DIR64} $? ${LIBS} -o $@

clean:
	rm -f *.o *.dll
	rm -f build/32/*
	rm -f build/64/*

32bit: build/32/xinput1_3.dll

64bit: build/64/xinput1_3.dll

all: build/32/xinput1_3.dll
